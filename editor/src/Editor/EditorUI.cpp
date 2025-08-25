#include "Editor/EditorUI.hpp"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "Graphics/SpriteRenderer.hpp"
#include "Core/Camera.hpp"
#include "Core/SceneSerialization.hpp"
#include "Core/Project.hpp"
#include <map>
#include <unordered_map>
#include <memory>
#include "Graphics/Texture.hpp"
#include <fstream>
#include <GLFW/glfw3.h>
#include "Engine.hpp"
#include "Core/Animator.hpp"
#include "Core/Rigidbody2D.hpp"
#include "Core/Collider2D.hpp"
#include "Core/Tilemap.hpp"
#include "Core/ScriptComponent.hpp"
#include <functional>
#include <filesystem>
#include <vector>
#include <string>
#include <unordered_set>
#include <algorithm>
#include "ImGuiColorTextEdit.h"
#if defined(__APPLE__)
extern "C" const char *Kiaak_ShowOpenFileDialog();
extern "C" const char *Kiaak_ShowOpenDirectoryDialog();
#endif

namespace Kiaak
{

    static GLFWwindow *currentWindow = nullptr;
    static std::vector<std::string> g_assetFiles;
    // Animation clip storage (editor only)
    static std::vector<EditorUI::AnimationClipInfo> g_animationClips;
    static int g_selectedClipIndex = -1;
    static bool g_openSheetEditor = false;
    static int g_sheetEditorClipIndex = -1;                                          // which clip is being edited
    static std::vector<int> g_tempSelection;                                         // selection order while editing
    static std::map<Core::GameObject *, int> g_objectClipAssignments;                // mapping sprite object -> clip index
    static std::unordered_map<std::string, std::shared_ptr<Texture>> g_textureCache; // editor preview cache
    static const char *kAnimationClipsFile = "animation_clips.json";                 // saved in working dir
    static const char *kAnimationAssignmentsFile = "animation_assignments.json";     // mapping objectID->clip index
    static std::unordered_map<uint32_t, int> g_pendingAssignments;                   // loaded IDs awaiting scene objects
    // Tilemap painting state
    static int g_tilemapPaintIndex = 0;
    static bool g_tilemapBrushErase = false;
    static bool g_tilemapPaintMode = false;    // must be toggled in inspector
    static bool g_tilemapColliderMode = false; // mutually exclusive with paint mode
    // Editor config persistence (e.g., texture filter mode)
    static const char *kEditorConfigFile = "editor_config.json"; // stored in project root if a project is open, else cwd
    static int g_savedFilterMode = -1;                           // track last saved to avoid redundant writes

    // Script editor state
    static bool g_scriptEditorOpen = false;
    static std::string g_openScriptPath; // relative or absolute
    static std::string g_scriptBuffer;   // saved text (authoritative on save)
    // Using embedded color text editor now
    static ImGuiColorTextEdit::TextEditor g_textEditor;
    static bool g_scriptDirty = false;
    static bool g_textEditorInitialized = false;

    void EditorUI::OpenScriptEditor(const std::string &scriptPath)
    {
        g_openScriptPath = scriptPath;
        g_scriptBuffer.clear();
        g_scriptDirty = false;
        // Load file
        std::ifstream ifs(scriptPath);
        if (!ifs.is_open() && Core::Project::HasPath())
        {
            // Try relative to project root
            std::ifstream ifs2(Core::Project::GetPath() + "/" + scriptPath);
            if (ifs2.is_open())
            {
                g_scriptBuffer.assign(std::istreambuf_iterator<char>(ifs2), std::istreambuf_iterator<char>());
            }
        }
        else if (ifs.is_open())
        {
            g_scriptBuffer.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
        }
        if (g_scriptBuffer.empty())
            g_scriptBuffer = "-- New Script\n\n";
        if (!g_textEditorInitialized)
        {
            g_textEditor.SetLanguageDefinition(ImGuiColorTextEdit::GetLuaLanguageDefinition());
            g_textEditorInitialized = true;
        }
        g_textEditor.SetText(g_scriptBuffer);
        // Engine only supports Lua; enable colorizer and set dark palette by default.
        g_textEditor.SetPalette(TextEditor::GetDarkPalette());
        g_textEditor.SetColorizerEnable(true);
        g_scriptEditorOpen = true;
    }

    bool EditorUI::IsScriptEditorOpen() { return g_scriptEditorOpen; }
    const std::string &EditorUI::GetOpenScriptPath() { return g_openScriptPath; }
    void EditorUI::CloseScriptEditor() { g_scriptEditorOpen = false; }

    void EditorUI::RenderScriptEditorOverlay()
    {
        if (!g_scriptEditorOpen)
            return;
        ImGuiIO &io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar;
        if (ImGui::Begin("Script Editor", nullptr, flags))
        {
            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("Save", "Ctrl+S", false, true))
                    {
                        // Save file
                        std::string full = g_openScriptPath;
                        if (Core::Project::HasPath() && !std::filesystem::exists(full))
                        {
                            std::string tryFull = Core::Project::GetPath() + "/" + g_openScriptPath;
                            full = tryFull;
                        }
                        std::ofstream ofs(full);
                        if (ofs.is_open())
                        {
                            g_scriptBuffer = g_textEditor.GetText();
                            ofs << g_scriptBuffer;
                            g_scriptDirty = false;
                        }
                    }
                    if (ImGui::MenuItem("Close", nullptr))
                    {
                        if (!g_scriptDirty)
                        {
                            g_scriptEditorOpen = false;
                        }
                        else
                        {
                            // simple prompt inline
                            ImGui::OpenPopup("Unsaved##script");
                        }
                    }
                    if (ImGui::MenuItem("Exit To Scene", "Esc"))
                    {
                        if (!g_scriptDirty)
                            g_scriptEditorOpen = false;
                        else
                            ImGui::OpenPopup("Unsaved##script");
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Run"))
                {
                    if (ImGui::MenuItem("Reload Script", nullptr, false, false))
                    {
                        // placeholder for future VM reload
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }
            // ESC key quick exit
            if (ImGui::IsKeyPressed(ImGuiKey_Escape))
            {
                if (!g_scriptDirty)
                    g_scriptEditorOpen = false;
                else
                    ImGui::OpenPopup("Unsaved##script");
            }
            if (ImGui::BeginPopupModal("Unsaved##script", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("Discard unsaved changes?");
                if (ImGui::Button("Discard"))
                {
                    g_scriptEditorOpen = false;
                    g_scriptDirty = false;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel"))
                {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
            ImGui::Separator();
            ImGui::TextUnformatted(g_openScriptPath.c_str());
            ImGui::Separator();
            // Integrated colored text editor (minimal implementation)
            g_textEditor.Render("##ScriptColorEditor", ImGui::GetContentRegionAvail());
            if (g_textEditor.IsTextChanged())
            {
                g_scriptDirty = true;
            }
            ImGui::Separator();
            if (ImGui::Button("Save"))
            {
                std::string full = g_openScriptPath;
                if (Core::Project::HasPath() && !std::filesystem::exists(full))
                    full = Core::Project::GetPath() + "/" + g_openScriptPath;
                std::ofstream ofs(full);
                if (ofs.is_open())
                {
                    g_scriptBuffer = g_textEditor.GetText();
                    ofs << g_scriptBuffer;
                    g_scriptDirty = false;
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Exit"))
            {
                if (!g_scriptDirty)
                    g_scriptEditorOpen = false;
                else
                    ImGui::OpenPopup("Unsaved##script");
            }
            // Keep authoritative buffer updated if user saved earlier via menu (optional sync each frame if not dirty)
            if (!g_scriptDirty)
            {
                // no-op; could sync here if external modifications were possible
            }
            // If buffer resized externally we would reallocate; simple approach keeps capacity safe.
        }
        ImGui::End();
    }

    static std::string GetEditorConfigPath()
    {
        if (Core::Project::HasPath())
            return Core::Project::GetPath() + "/" + kEditorConfigFile;
        return std::string(kEditorConfigFile);
    }

    static void SaveEditorConfig()
    {
        int currentFilter = (int)Texture::GetGlobalFilterMode();
        if (currentFilter == g_savedFilterMode)
            return; // no change
        std::ofstream out(GetEditorConfigPath(), std::ios::trunc);
        if (!out.is_open())
            return;
        // Minimal JSON with single field
        out << "{\n  \"textureFilter\": " << currentFilter << "\n}";
        g_savedFilterMode = currentFilter;
    }

    static void LoadEditorConfig()
    {
        std::ifstream in(GetEditorConfigPath());
        if (!in.is_open())
            return;
        std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        // very small parse: look for "textureFilter" then a number
        auto pos = content.find("textureFilter");
        if (pos == std::string::npos)
            return;
        pos = content.find(':', pos);
        if (pos == std::string::npos)
            return;
        ++pos;
        while (pos < content.size() && isspace((unsigned char)content[pos]))
            ++pos;
        int val = 0;
        bool neg = false;
        if (pos < content.size() && content[pos] == '-')
        {
            neg = true; // should not happen, but handle anyway
            ++pos;
        }
        while (pos < content.size() && isdigit((unsigned char)content[pos]))
        {
            val = val * 10 + (content[pos] - '0');
            ++pos;
        }
        if (neg)
            val = -val;
        if (val == (int)Texture::FilterMode::Linear || val == (int)Texture::FilterMode::Nearest)
        {
            Texture::SetGlobalFilterMode(static_cast<Texture::FilterMode>(val));
            g_savedFilterMode = val; // mark as loaded
        }
    }

    // Very tiny ad-hoc JSON (array of objects) writer (no escaping for simplicity)
    static void WriteClipsJSON(const std::vector<EditorUI::AnimationClipInfo> &clips, std::ostream &out)
    {
        out << "[\n";
        for (size_t i = 0; i < clips.size(); ++i)
        {
            auto &c = clips[i];
            out << "  {\n";
            out << "    \"name\": \"" << c.name << "\",\n";
            out << "    \"texturePath\": \"" << c.texturePath << "\",\n";
            out << "    \"hFrames\": " << c.hFrames << ",\n";
            out << "    \"vFrames\": " << c.vFrames << ",\n";
            out << "    \"cellWidth\": " << c.cellWidth << ",\n";
            out << "    \"cellHeight\": " << c.cellHeight << ",\n";
            out << "    \"sequence\": [";
            for (size_t s = 0; s < c.sequence.size(); ++s)
            {
                out << c.sequence[s];
                if (s + 1 < c.sequence.size())
                    out << ",";
            }
            out << "],\n";
            out << "    \"fps\": " << c.fps << ",\n";
            out << "    \"autoPlay\": " << (c.autoPlay ? 1 : 0) << "\n";
            out << "  }" << (i + 1 < clips.size() ? "," : "") << "\n";
        }
        out << "]";
    }

    // Minimal parser: expects same format produced above. Skips errors.
    static void LoadClipsJSON(std::istream &in, std::vector<EditorUI::AnimationClipInfo> &outClips)
    {
        std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        size_t pos = 0;
        auto skipWS = [&]()
        { while (pos < content.size() && isspace((unsigned char)content[pos])) ++pos; };
        auto parseNumber = [&]() -> double
        {
            int sign = 1;
            if (pos < content.size() && content[pos] == '-')
            {
                sign = -1;
                ++pos;
            }
            double num = 0.0;
            while (pos < content.size() && isdigit((unsigned char)content[pos]))
            {
                num = num * 10 + (content[pos] - '0');
                ++pos;
            }
            if (pos < content.size() && content[pos] == '.')
            {
                ++pos;
                double frac = 0.0, base = 0.1;
                while (pos < content.size() && isdigit((unsigned char)content[pos]))
                {
                    frac += (content[pos] - '0') * base;
                    base *= 0.1;
                    ++pos;
                }
                num += frac;
            }
            return num * sign;
        };
        skipWS();
        if (pos >= content.size() || content[pos] != '[')
            return;
        ++pos; // skip [
        while (true)
        {
            skipWS();
            if (pos >= content.size())
                break;
            if (content[pos] == ']')
            {
                ++pos;
                break;
            }
            if (content[pos] != '{')
            {
                ++pos;
                continue;
            }
            ++pos; // object
            EditorUI::AnimationClipInfo clip;
            bool any = false;
            while (true)
            {
                skipWS();
                if (pos >= content.size())
                    break;
                if (content[pos] == '}')
                {
                    ++pos;
                    break;
                }
                if (content[pos] != '"')
                {
                    ++pos;
                    continue;
                }
                size_t kStart = ++pos;
                while (pos < content.size() && content[pos] != '"')
                    ++pos;
                if (pos >= content.size())
                    break;
                std::string key = content.substr(kStart, pos - kStart);
                ++pos;
                skipWS();
                if (pos < content.size() && content[pos] == ':')
                    ++pos;
                skipWS();
                if (key == "name" || key == "texturePath")
                {
                    if (pos < content.size() && content[pos] == '"')
                    {
                        size_t vStart = ++pos;
                        while (pos < content.size() && content[pos] != '"')
                            ++pos;
                        if (pos >= content.size())
                            break;
                        std::string val = content.substr(vStart, pos - vStart);
                        ++pos;
                        if (key == "name")
                            clip.name = val;
                        else
                            clip.texturePath = val;
                    }
                }
                else if (key == "hFrames" || key == "vFrames" || key == "cellWidth" || key == "cellHeight")
                {
                    int sign = 1;
                    if (pos < content.size() && content[pos] == '-')
                    {
                        sign = -1;
                        ++pos;
                    }
                    int num = 0;
                    while (pos < content.size() && isdigit((unsigned char)content[pos]))
                    {
                        num = num * 10 + (content[pos] - '0');
                        ++pos;
                    }
                    num *= sign;
                    if (key == "hFrames")
                        clip.hFrames = num;
                    else if (key == "vFrames")
                        clip.vFrames = num;
                    else if (key == "cellWidth")
                        clip.cellWidth = num;
                    else
                        clip.cellHeight = num;
                }
                else if (key == "sequence")
                {
                    skipWS();
                    if (pos < content.size() && content[pos] == '[')
                    {
                        ++pos;
                        skipWS();
                        while (pos < content.size() && content[pos] != ']')
                        {
                            int sign = 1;
                            if (content[pos] == '-')
                            {
                                sign = -1;
                                ++pos;
                            }
                            int num = 0;
                            while (pos < content.size() && isdigit((unsigned char)content[pos]))
                            {
                                num = num * 10 + (content[pos] - '0');
                                ++pos;
                            }
                            clip.sequence.push_back(num * sign);
                            skipWS();
                            if (pos < content.size() && content[pos] == ',')
                            {
                                ++pos;
                                skipWS();
                            }
                        }
                        if (pos < content.size() && content[pos] == ']')
                            ++pos;
                    }
                }
                else if (key == "fps")
                {
                    clip.fps = (float)parseNumber();
                }
                else if (key == "autoPlay")
                {
                    clip.autoPlay = parseNumber() != 0.0;
                }
                any = true;
                skipWS();
                if (pos < content.size() && content[pos] == ',')
                {
                    ++pos;
                }
            }
            if (any)
            {
                clip.dirty = false;
                outClips.push_back(std::move(clip));
            }
            skipWS();
            if (pos < content.size() && content[pos] == ',')
            {
                ++pos;
                continue;
            }
        }
    }

    static std::shared_ptr<Texture> GetOrLoadTexture(const std::string &path)
    {
        if (path.empty())
            return nullptr;
        // Resolve relative paths against project assets folder
        std::string resolved = path;
        if (!std::filesystem::exists(resolved))
        {
            if (Core::Project::HasPath())
            {
                auto tryPath = Core::Project::GetAssetsPath() + "/" + path;
                if (std::filesystem::exists(tryPath))
                    resolved = tryPath;
            }
            else
            {
                std::string tryPath = std::string("assets/") + path;
                if (std::filesystem::exists(tryPath))
                    resolved = tryPath;
            }
        }
        auto cacheKey = resolved;
        auto it = g_textureCache.find(cacheKey);
        if (it != g_textureCache.end())
            return it->second;
        auto tex = std::make_shared<Texture>(resolved);
        if (!tex->IsValid())
            return nullptr;
        g_textureCache[cacheKey] = tex;
        return tex;
    }
    int EditorUI::GetActiveTilemapPaintIndex() { return g_tilemapPaintIndex; }
    void EditorUI::SetActiveTilemapPaintIndex(int index)
    {
        if (index >= 0)
            g_tilemapPaintIndex = index;
    }
    bool EditorUI::IsTilemapPaintMode() { return g_tilemapPaintMode; }
    void EditorUI::SetTilemapPaintMode(bool v) { g_tilemapPaintMode = v; }
    bool EditorUI::IsTilemapColliderMode() { return g_tilemapColliderMode; }
    void EditorUI::SetTilemapColliderMode(bool v) { g_tilemapColliderMode = v; }
    static double g_lastAssetRefreshCheck = 0.0; // seconds since start
    static const char *kAssetDir = "assets";     // fallback when no project

    static std::string CurrentAssetRoot()
    {
        if (Core::Project::HasPath())
            return Core::Project::GetAssetsPath();
        return kAssetDir;
    }

    void EditorUI::RefreshAssetList(bool force)
    {
        namespace fs = std::filesystem;
        if (!force)
        {
            // Basic throttle: only rescan if > 1s since last attempt
            double now = ImGui::GetTime();
            if (now - g_lastAssetRefreshCheck < 1.0)
                return;
            g_lastAssetRefreshCheck = now;
        }
        g_assetFiles.clear();
        auto root = CurrentAssetRoot();
        try
        {
            if (fs::exists(root) && fs::is_directory(root))
            {
                for (auto &entry : fs::recursive_directory_iterator(root))
                {
                    if (!entry.is_regular_file())
                        continue;
                    auto path = entry.path();
                    // Accept common image extensions
                    auto ext = path.extension().string();
                    for (auto &c : ext)
                        c = (char)tolower(c);
                    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga")
                        g_assetFiles.push_back(path.generic_string());
                }
            }
        }
        catch (...)
        {
        }
        std::sort(g_assetFiles.begin(), g_assetFiles.end());
    }

    const std::vector<std::string> &EditorUI::GetAssetFiles()
    {
        if (g_assetFiles.empty())
            RefreshAssetList(true);
        return g_assetFiles;
    }

    void EditorUI::Initialize()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        // Docking disabled: current ImGui build lacks docking branch.

        ImGui::StyleColorsDark(); // start from dark base

        // --- Modern Theme Override ---
        {
            ImGuiStyle &style = ImGui::GetStyle();
            style.WindowRounding = 6.0f;
            style.FrameRounding = 5.0f;
            style.ChildRounding = 5.0f;
            style.PopupRounding = 5.0f;
            style.GrabRounding = 4.0f;
            style.ScrollbarRounding = 6.0f;
            style.TabRounding = 5.0f;
            style.FrameBorderSize = 1.0f;
            style.WindowBorderSize = 1.0f;
            style.TabBorderSize = 0.0f;
            style.SeparatorTextBorderSize = 1.0f;
            style.WindowPadding = ImVec2(10, 10);
            style.FramePadding = ImVec2(10, 6);
            style.ItemSpacing = ImVec2(8, 6);
            style.ItemInnerSpacing = ImVec2(6, 4);

            ImVec4 bg1 = ImVec4(0.11f, 0.12f, 0.14f, 1.0f);
            ImVec4 bg2 = ImVec4(0.15f, 0.16f, 0.19f, 1.0f);
            ImVec4 bg3 = ImVec4(0.20f, 0.21f, 0.24f, 1.0f);
            ImVec4 accent = ImVec4(0.05f, 0.55f, 0.78f, 1.0f);    // primary accent (teal)
            ImVec4 accentHi = ImVec4(0.15f, 0.65f, 0.88f, 1.0f);  // hover accent
            ImVec4 accentAct = ImVec4(0.02f, 0.45f, 0.68f, 1.0f); // active accent
            ImVec4 text = ImVec4(0.93f, 0.94f, 0.95f, 1.0f);
            ImVec4 textDim = ImVec4(0.55f, 0.58f, 0.62f, 1.0f);

            auto &colors = style.Colors;
            colors[ImGuiCol_Text] = text;
            colors[ImGuiCol_TextDisabled] = textDim;
            colors[ImGuiCol_WindowBg] = bg1;
            colors[ImGuiCol_ChildBg] = bg1;
            colors[ImGuiCol_PopupBg] = bg1;
            colors[ImGuiCol_Border] = ImVec4(0.27f, 0.29f, 0.33f, 1.0f);
            colors[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0.0f);
            colors[ImGuiCol_FrameBg] = bg2;
            colors[ImGuiCol_FrameBgHovered] = bg3;
            colors[ImGuiCol_FrameBgActive] = bg3;
            colors[ImGuiCol_TitleBg] = bg1;
            colors[ImGuiCol_TitleBgActive] = bg2;
            colors[ImGuiCol_TitleBgCollapsed] = bg1;
            colors[ImGuiCol_MenuBarBg] = bg2;
            colors[ImGuiCol_ScrollbarBg] = bg1;
            colors[ImGuiCol_ScrollbarGrab] = bg2;
            colors[ImGuiCol_ScrollbarGrabHovered] = bg3;
            colors[ImGuiCol_ScrollbarGrabActive] = bg3;
            colors[ImGuiCol_CheckMark] = accent;
            colors[ImGuiCol_SliderGrab] = accent;
            colors[ImGuiCol_SliderGrabActive] = accentHi;
            colors[ImGuiCol_Button] = bg2;
            colors[ImGuiCol_ButtonHovered] = bg3;
            colors[ImGuiCol_ButtonActive] = accentAct;
            colors[ImGuiCol_Header] = accent;
            colors[ImGuiCol_HeaderHovered] = accentHi;
            colors[ImGuiCol_HeaderActive] = accentAct;
            colors[ImGuiCol_Separator] = ImVec4(0.30f, 0.32f, 0.36f, 1.0f);
            colors[ImGuiCol_SeparatorHovered] = accentHi;
            colors[ImGuiCol_SeparatorActive] = accentAct;
            colors[ImGuiCol_ResizeGrip] = bg2;
            colors[ImGuiCol_ResizeGripHovered] = accentHi;
            colors[ImGuiCol_ResizeGripActive] = accentAct;
            colors[ImGuiCol_Tab] = bg2;
            colors[ImGuiCol_TabHovered] = accentHi;
            colors[ImGuiCol_TabActive] = accent;
            colors[ImGuiCol_TabUnfocused] = bg2;
            colors[ImGuiCol_TabUnfocusedActive] = accent;
            colors[ImGuiCol_TableHeaderBg] = bg2;
            colors[ImGuiCol_TableBorderStrong] = ImVec4(0.25f, 0.27f, 0.30f, 1.0f);
            colors[ImGuiCol_TableBorderLight] = ImVec4(0.17f, 0.18f, 0.20f, 1.0f);
            colors[ImGuiCol_NavHighlight] = accentHi;
            colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1, 1, 1, 0.70f);
            colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0, 0, 0, 0.20f);
            colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0, 0, 0, 0.35f);
        }

        // --- Font Loading (Primary + Optional Icon Merge) ---
        // Users can drop fonts into assets/fonts/ (e.g., JetBrainsMono-Regular.ttf, MaterialIcons.ttf)
        {
            const char *monoPath = "assets/fonts/JetBrainsMono-Regular.ttf";  // optional
            const char *iconPath = "assets/fonts/MaterialSymbolsRounded.ttf"; // user-provided (not bundled).
            float baseSize = 16.0f;
            if (std::filesystem::exists(monoPath))
            {
                ImFontConfig cfg;
                cfg.OversampleH = 3;
                cfg.OversampleV = 2;
                cfg.PixelSnapH = false;
                cfg.GlyphOffset = ImVec2(0, 0);
                io.Fonts->AddFontFromFileTTF(monoPath, baseSize, &cfg);
            }
            else
            {
                io.Fonts->AddFontDefault();
            }
            if (std::filesystem::exists(iconPath))
            {
                static const ImWchar iconRanges[] = {0xE000, 0xF8FF, 0}; // Private Use Area
                ImFontConfig icfg;
                icfg.MergeMode = true;
                icfg.PixelSnapH = true;
                icfg.GlyphMinAdvanceX = 13.0f;
                io.Fonts->AddFontFromFileTTF(iconPath, baseSize + 2.0f, &icfg, iconRanges);
            }
        }

        // Load editor config (e.g., texture filter) before loading assets so textures can be updated immediately
        LoadEditorConfig();

        std::ifstream in(kAnimationClipsFile);
        if (in.good())
        {
            LoadClipsJSON(in, g_animationClips);
        }
        // Load assignments file
        g_pendingAssignments.clear();
        std::ifstream ain(kAnimationAssignmentsFile);
        if (ain.good())
        {
            std::string content((std::istreambuf_iterator<char>(ain)), std::istreambuf_iterator<char>());
            size_t pos = 0;
            auto ws = [&]()
            { while (pos<content.size() && isspace((unsigned char)content[pos])) ++pos; };
            ws();
            if (pos < content.size() && content[pos] == '{')
            {
                ++pos;
                while (true)
                {
                    ws();
                    if (pos >= content.size())
                        break;
                    if (content[pos] == '}')
                    {
                        ++pos;
                        break;
                    }
                    if (content[pos] == '"')
                    {
                        size_t s = ++pos;
                        while (pos < content.size() && content[pos] != '"')
                            ++pos;
                        if (pos >= content.size())
                            break;
                        std::string key = content.substr(s, pos - s);
                        ++pos;
                        ws();
                        if (pos < content.size() && content[pos] == ':')
                            ++pos;
                        ws();
                        int sign = 1;
                        if (content[pos] == '-')
                        {
                            sign = -1;
                            ++pos;
                        }
                        int num = 0;
                        while (pos < content.size() && isdigit((unsigned char)content[pos]))
                        {
                            num = num * 10 + (content[pos] - '0');
                            ++pos;
                        }
                        num *= sign;
                        try
                        {
                            uint32_t id = (uint32_t)std::stoul(key);
                            g_pendingAssignments[id] = num;
                        }
                        catch (...)
                        {
                        }
                        ws();
                        if (pos < content.size() && content[pos] == ',')
                        {
                            ++pos;
                            continue;
                        }
                    }
                    else
                    {
                        ++pos;
                    }
                }
            }
        }
    }

    void EditorUI::ApplyPendingAnimationAssignments(Core::Scene *scene)
    {
        if (!scene || g_pendingAssignments.empty())
            return;
        auto objects = scene->GetAllGameObjects();
        for (auto *go : objects)
        {
            if (!go)
                continue;
            auto it = g_pendingAssignments.find(go->GetID());
            if (it != g_pendingAssignments.end())
            {
                g_objectClipAssignments[go] = it->second;
                // Auto-add animator and set clip
                if (!go->GetComponent<Core::Animator>())
                    go->AddComponent<Core::Animator>();
                if (auto *anim = go->GetComponent<Core::Animator>())
                    anim->SetClipIndex(it->second);
            }
        }
    }

    void EditorUI::InitializeForWindow(GLFWwindow *window)
    {
        currentWindow = window;
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330 core");
    }

    void EditorUI::Shutdown()
    {
        std::ofstream out(kAnimationClipsFile);
        if (out.good())
            WriteClipsJSON(g_animationClips, out);
        // Save object assignments (by ID)
        if (!g_objectClipAssignments.empty())
        {
            std::ofstream aout(kAnimationAssignmentsFile);
            if (aout.good())
            {
                aout << "{\n";
                size_t count = 0;
                size_t total = g_objectClipAssignments.size();
                for (auto &pair : g_objectClipAssignments)
                {
                    if (!pair.first)
                        continue;
                    aout << "  \"" << pair.first->GetID() << "\": " << pair.second;
                    if (++count < total)
                        aout << ",";
                    aout << "\n";
                }
                aout << "}";
            }
        }
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void EditorUI::BeginFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void EditorUI::EndFrame()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    // Unified hierarchy with scenes + objects
    void EditorUI::RenderSceneHierarchy(Core::SceneManager *sceneManager, Core::Scene *&activeScene, Core::GameObject *&selectedObject)
    {
        constexpr float kTopBarHeight = 34.0f; // must match top bar height
        ImGui::SetNextWindowPos(ImVec2(0, kTopBarHeight));
        ImGui::SetNextWindowSize(ImVec2(320, ImGui::GetIO().DisplaySize.y - 200 - kTopBarHeight));
        if (ImGui::Begin("Scene Hierarchy", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
        {
            if (sceneManager)
            {
                // Context menu on empty space inside hierarchy window
                if (ImGui::BeginPopupContextWindow("HierarchyContext", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
                {
                    if (ImGui::MenuItem("New Animation"))
                    {
                        // Create empty clip (no auto sheet popup)
                        EditorUI::AnimationClipInfo clip;
                        clip.name = "Anim" + std::to_string((int)g_animationClips.size() + 1);
                        g_animationClips.push_back(clip);
                        g_selectedClipIndex = (int)g_animationClips.size() - 1;
                        // Persist immediately
                        std::ofstream out(kAnimationClipsFile);
                        if (out.good())
                            WriteClipsJSON(g_animationClips, out);
                    }
                    if (ImGui::MenuItem("New Scene"))
                    {
                        static int sceneCounter = 1;
                        std::string name = "Scene" + std::to_string(sceneCounter++);
                        if (sceneManager->CreateScene(name))
                        {
                            sceneManager->SwitchToScene(name);
                            activeScene = sceneManager->GetCurrentScene();
                            selectedObject = nullptr;
                            if (Core::Project::HasPath())
                            {
                                auto sn = sceneManager->GetSceneName(activeScene);
                                if (!sn.empty())
                                    Core::SceneSerialization::SaveSceneToFile(activeScene, Core::Project::GetScenesPath() + "/" + sn + ".scene");
                            }
                        }
                    }
                    if (ImGui::MenuItem("Create Camera"))
                    {
                        if (activeScene)
                        {
                            auto *go = activeScene->CreateGameObject("Camera");
                            auto *cam = go->AddComponent<Core::Camera>();
                            cam->SetOrthographicSize(10.0f);
                            cam->SetZoom(1.0f);
                            // Do not globally SetActive() here (editor camera remains active in editor mode)
                            activeScene->SetDesignatedCamera(cam);
                            selectedObject = go;
                            if (Core::Project::HasPath())
                            {
                                auto sn = sceneManager->GetSceneName(activeScene);
                                if (!sn.empty())
                                    Core::SceneSerialization::SaveSceneToFile(activeScene, Core::Project::GetScenesPath() + "/" + sn + ".scene");
                            }
                        }
                    }
                    if (ImGui::MenuItem("Create Tilemap"))
                    {
                        if (activeScene)
                        {
                            auto *go = activeScene->CreateGameObject("Tilemap");
                            auto *tm = go->AddComponent<Core::Tilemap>();
                            if (tm)
                            {
                                tm->SetMapSize(8, 8);
                                tm->SetTileSize(1.0f, 1.0f);
                                tm->SetTileset("", 1, 1); // empty path; user sets later
                            }
                            selectedObject = go;
                            if (Core::Project::HasPath())
                            {
                                auto sn = sceneManager->GetSceneName(activeScene);
                                if (!sn.empty())
                                    Core::SceneSerialization::SaveSceneToFile(activeScene, Core::Project::GetScenesPath() + "/" + sn + ".scene");
                            }
                        }
                    }
                    if (ImGui::BeginMenu("Create Sprite"))
                    {
                        RefreshAssetList();
                        const auto &assets = GetAssetFiles();
                        if (assets.empty())
                        {
                            ImGui::MenuItem("<No image assets found>", nullptr, false, false);
                        }
                        for (auto &path : assets)
                        {
                            std::filesystem::path p(path);
                            std::string fname = p.filename().string();
                            if (ImGui::MenuItem(fname.c_str()))
                            {
                                if (activeScene)
                                {
                                    auto *go = activeScene->CreateGameObject("Sprite");
                                    go->AddComponent<Graphics::SpriteRenderer>(path.c_str());
                                    go->GetTransform()->SetPosition(0, 0, 0);
                                    selectedObject = go;
                                    if (Core::Project::HasPath())
                                    {
                                        auto sn = sceneManager->GetSceneName(activeScene);
                                        if (!sn.empty())
                                            Core::SceneSerialization::SaveSceneToFile(activeScene, Core::Project::GetScenesPath() + "/" + sn + ".scene");
                                    }
                                }
                            }
                            if (ImGui::IsItemHovered())
                                ImGui::SetTooltip("%s", path.c_str());
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndPopup();
                }

                // Collect any deletions requested this frame (defer actual removal until after drawing)
                std::vector<Core::GameObject *> pendingDelete;

                auto sceneNames = sceneManager->GetSceneNames();
                std::vector<std::string> scenesPendingDelete; // defer until after tree loop
                for (auto &sceneName : sceneNames)
                {
                    Core::Scene *scene = sceneManager->GetScene(sceneName);
                    bool isActiveScene = (scene == activeScene);
                    ImGuiTreeNodeFlags sceneFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
                    std::string label = std::string(isActiveScene ? "[Active] " : "") + sceneName;
                    bool openScene = ImGui::TreeNodeEx((void *)scene, sceneFlags, "%s", label.c_str());
                    if (ImGui::IsItemClicked())
                    {
                        sceneManager->SwitchToScene(sceneName);
                        activeScene = scene;
                        selectedObject = nullptr;
                    }
                    // Scene level context menu
                    if (ImGui::BeginPopupContextItem())
                    {
                        if (ImGui::MenuItem("Set Active"))
                        {
                            sceneManager->SwitchToScene(sceneName);
                            activeScene = scene;
                            selectedObject = nullptr;
                        }
                        bool canDelete = (sceneManager->GetSceneCount() > 1);
                        if (ImGui::MenuItem("Delete Scene", nullptr, false, canDelete))
                        {
                            scenesPendingDelete.push_back(sceneName);
                        }
                        if (!canDelete && ImGui::IsItemHovered())
                            ImGui::SetTooltip("Need at least one scene");
                        ImGui::EndPopup();
                    }

                    if (openScene && scene)
                    {
                        const auto &all = scene->GetAllGameObjects();
                        std::function<void(Core::GameObject *)> drawNode = [&](Core::GameObject *node)
                        {
                            // Skip editor camera object(s) (any name starting with "EditorCamera")
                            if (node->GetName().rfind("EditorCamera", 0) == 0)
                                return; // do not render this node at all

                            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
                            if (node->GetChildren().empty())
                                flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                            if (selectedObject == node)
                                flags |= ImGuiTreeNodeFlags_Selected;
                            // Draw tree node (label used for ID only so we can custom-draw name + icon)
                            bool open = ImGui::TreeNodeEx((void *)node, flags, "%s", node->GetName().c_str());
                            // Draw icon overlay (camera or sprite) to the left of text inside item rect
                            ImVec2 itemMin = ImGui::GetItemRectMin();
                            ImVec2 itemMax = ImGui::GetItemRectMax();
                            float iconSize = (itemMax.y - itemMin.y) * 0.55f; // square that fits line height
                            float iconPadY = (itemMax.y - itemMin.y - iconSize) * 0.5f;
                            float iconPadX = 4.0f; // small left padding after arrow
                            // Compute where ImGui placed text start: we approximate by itemMin.x + arrow+frame padding
                            // We'll shift left a bit from text start; simpler: draw at itemMin.x + iconPadX
                            ImVec2 iconMin(itemMin.x + iconPadX, itemMin.y + iconPadY);
                            ImVec2 iconMax(iconMin.x + iconSize, iconMin.y + iconSize);
                            auto *dl = ImGui::GetWindowDrawList();
                            bool hasCamera = node->GetComponent<Core::Camera>() != nullptr;
                            bool hasSprite = node->GetComponent<Graphics::SpriteRenderer>() != nullptr;
                            if (hasCamera || hasSprite)
                            {
                                if (hasCamera)
                                {
                                    // Warmer camera icon
                                    ImU32 bodyCol = IM_COL32(227, 166, 64, 255);  // amber
                                    ImU32 lensCol = IM_COL32(255, 244, 220, 255); // warm light
                                    dl->AddRectFilled(iconMin, iconMax, bodyCol, 2.5f);
                                    float lensRadius = iconSize * 0.30f;
                                    ImVec2 lensCenter(iconMin.x + iconSize * 0.63f, iconMin.y + iconSize * 0.50f);
                                    dl->AddCircleFilled(lensCenter, lensRadius, lensCol, 14);
                                    ImVec2 vf0(iconMin.x - iconSize * 0.24f, iconMin.y + iconSize * 0.18f);
                                    ImVec2 vf1(iconMin.x, iconMin.y + iconSize * 0.58f);
                                    dl->AddRectFilled(vf0, vf1, bodyCol, 2.0f);
                                }
                                else if (hasSprite)
                                {
                                    // Warmer sprite icon
                                    ImU32 fillCol = IM_COL32(255, 138, 101, 255);  // coral
                                    ImU32 outlineCol = IM_COL32(217, 90, 58, 255); // deeper outline
                                    float cx = (iconMin.x + iconMax.x) * 0.5f;
                                    float cy = (iconMin.y + iconMax.y) * 0.5f;
                                    float r = iconSize * 0.46f;
                                    ImVec2 p0(cx, cy - r);
                                    ImVec2 p1(cx + r, cy);
                                    ImVec2 p2(cx, cy + r);
                                    ImVec2 p3(cx - r, cy);
                                    dl->AddQuadFilled(p0, p1, p2, p3, fillCol);
                                    dl->AddQuad(p0, p1, p2, p3, outlineCol, 1.2f);
                                }
                            }
                            if (ImGui::IsItemClicked())
                                selectedObject = node;

                            // Per-object context menu
                            if (ImGui::BeginPopupContextItem())
                            {
                                bool hasSprite = node->GetComponent<Graphics::SpriteRenderer>() != nullptr;
                                bool hasRB = node->GetComponent<Core::Rigidbody2D>() != nullptr;
                                bool hasBox = node->GetComponent<Core::BoxCollider2D>() != nullptr;
                                bool hasTilemap = node->GetComponent<Core::Tilemap>() != nullptr;
                                bool hasScript = node->GetComponent<Core::ScriptComponent>() != nullptr;
                                if (ImGui::MenuItem("Add Rigidbody 2D", nullptr, false, hasSprite && !hasRB))
                                {
                                    auto *rb = node->AddComponent<Core::Rigidbody2D>();
                                    if (rb)
                                    {
                                        rb->SetGravityScale(1.0f);
                                    }
                                    if (sceneManager && Core::Project::HasPath())
                                    {
                                        // Save owning scene
                                        for (auto &nm : sceneManager->GetSceneNames())
                                        {
                                            auto *sc = sceneManager->GetScene(nm);
                                            if (!sc)
                                                continue;
                                            for (auto *go : sc->GetAllGameObjects())
                                            {
                                                if (go == node)
                                                {
                                                    Core::SceneSerialization::SaveSceneToFile(sc, Core::Project::GetScenesPath() + "/" + nm + ".scene");
                                                    goto add_rb_saved;
                                                }
                                            }
                                        }
                                    }
                                add_rb_saved:;
                                }
                                if (ImGui::MenuItem("Add Box Collider 2D", nullptr, false, hasSprite && !hasBox))
                                {
                                    node->AddComponent<Core::BoxCollider2D>();
                                    if (sceneManager && Core::Project::HasPath())
                                    {
                                        for (auto &nm : sceneManager->GetSceneNames())
                                        {
                                            auto *sc = sceneManager->GetScene(nm);
                                            if (!sc)
                                                continue;
                                            for (auto *go : sc->GetAllGameObjects())
                                            {
                                                if (go == node)
                                                {
                                                    Core::SceneSerialization::SaveSceneToFile(sc, Core::Project::GetScenesPath() + "/" + nm + ".scene");
                                                    goto add_box_saved;
                                                }
                                            }
                                        }
                                    }
                                add_box_saved:;
                                }
                                if (ImGui::MenuItem("Add Tilemap", nullptr, false, !hasTilemap))
                                {
                                    auto *tm = node->AddComponent<Core::Tilemap>();
                                    if (tm)
                                    {
                                        tm->SetMapSize(8, 8);
                                        tm->SetTileSize(1.0f, 1.0f);
                                        tm->SetTileset("", 1, 1);
                                    }
                                    if (sceneManager && Core::Project::HasPath())
                                    {
                                        for (auto &nm : sceneManager->GetSceneNames())
                                        {
                                            auto *sc = sceneManager->GetScene(nm);
                                            if (!sc)
                                                continue;
                                            for (auto *go : sc->GetAllGameObjects())
                                            {
                                                if (go == node)
                                                {
                                                    Core::SceneSerialization::SaveSceneToFile(sc, Core::Project::GetScenesPath() + "/" + nm + ".scene");
                                                    goto add_tilemap_saved;
                                                }
                                            }
                                        }
                                    }
                                add_tilemap_saved:;
                                }
                                if (ImGui::MenuItem("Add Script", nullptr, false, !hasScript))
                                {
                                    auto *scpt = node->AddComponent<Core::ScriptComponent>();
                                    if (scpt)
                                    {
                                        if (Core::Project::HasPath())
                                        {
                                            auto scriptsDir = Core::Project::GetPath() + "/scripts";
                                            std::error_code ec;
                                            std::filesystem::create_directories(scriptsDir, ec);
                                            std::string base = node->GetName();
                                            if (base.empty())
                                                base = "Script";
                                            std::string fname = base + "_Script.lua";
                                            std::string full = scriptsDir + "/" + fname;
                                            int counter = 1;
                                            while (std::filesystem::exists(full))
                                            {
                                                fname = base + "_Script" + std::to_string(counter++) + ".lua";
                                                full = scriptsDir + "/" + fname;
                                            }
                                            std::ofstream ofs(full);
                                            if (ofs.is_open())
                                            {
                                                ofs << "-- Auto-generated script for object: " << node->GetName() << "\n";
                                                ofs << "function OnStart()\n    -- init\nend\n\nfunction OnUpdate(dt)\n    -- logic\nend\n";
                                            }
                                            scpt->SetScriptPath("scripts/" + fname);
                                            if (sceneManager)
                                            {
                                                for (auto &nm : sceneManager->GetSceneNames())
                                                {
                                                    auto *sc = sceneManager->GetScene(nm);
                                                    if (!sc)
                                                        continue;
                                                    for (auto *go : sc->GetAllGameObjects())
                                                    {
                                                        if (go == node)
                                                        {
                                                            Core::SceneSerialization::SaveSceneToFile(sc, Core::Project::GetScenesPath() + "/" + nm + ".scene");
                                                            goto add_script_done;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                add_script_done:;
                                }
                                if (ImGui::MenuItem("Delete"))
                                {
                                    // Mark for deferred deletion
                                    pendingDelete.push_back(node);
                                    if (selectedObject == node)
                                        selectedObject = nullptr;
                                    ImGui::EndPopup();
                                    return; // don't recurse into (soon-to-be deleted) children
                                }
                                ImGui::EndPopup();
                            }

                            if (open && !node->GetChildren().empty())
                            {
                                for (auto *child : node->GetChildren())
                                    drawNode(child);
                                ImGui::TreePop();
                            }
                        };
                        for (auto *obj : all)
                            if (obj->GetParent() == nullptr)
                                drawNode(obj);
                        ImGui::TreePop();
                    }
                }

                // Execute deletions after tree traversal to avoid use-after-free during ImGui draw.
                if (!pendingDelete.empty() && activeScene)
                {
                    for (auto *go : pendingDelete)
                    {
                        activeScene->RemoveGameObject(go->GetName());
                    }
                    if (Core::Project::HasPath())
                    {
                        auto sn = sceneManager->GetSceneName(activeScene);
                        if (!sn.empty())
                            Core::SceneSerialization::SaveSceneToFile(activeScene, Core::Project::GetScenesPath() + "/" + sn + ".scene");
                    }
                }
                if (!scenesPendingDelete.empty())
                {
                    for (auto &snDel : scenesPendingDelete)
                    {
                        bool wasCurrent = (sceneManager->GetCurrentScene() && sceneManager->GetCurrentScene() == sceneManager->GetScene(snDel));
                        if (sceneManager->DeleteScene(snDel))
                        {
                            if (wasCurrent)
                                selectedObject = nullptr;
                            // Remove file on disk
                            if (Core::Project::HasPath())
                            {
                                std::filesystem::remove(Core::Project::GetScenesPath() + "/" + snDel + ".scene");
                            }
                        }
                    }
                    // No bulk save needed; each deletion handled.
                }
            }
        }
        ImGui::End();
    }

    void EditorUI::RenderInspector(Core::SceneManager *sceneManager, Core::GameObject *selectedObject)
    {
        constexpr float kTopBarHeight = 34.0f; // must match top bar height
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 300, kTopBarHeight));
        ImGui::SetNextWindowSize(ImVec2(300, ImGui::GetIO().DisplaySize.y - 200 - kTopBarHeight));

        if (ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
        {
            if (selectedObject)
            {
                // Editable name at top
                static char nameBuffer[256];
                // Copy current name into buffer each frame if different (avoid losing user edits mid-typing?)
                // Simple approach: if buffer doesn't match selected object's name and item not active, sync.
                if (!ImGui::IsAnyItemActive())
                {
                    std::string currentName = selectedObject->GetName();
                    if (strncmp(nameBuffer, currentName.c_str(), sizeof(nameBuffer)) != 0)
                    {
                        strncpy(nameBuffer, currentName.c_str(), sizeof(nameBuffer) - 1);
                        nameBuffer[sizeof(nameBuffer) - 1] = '\0';
                    }
                }
                ImGui::Text("Name");
                ImGui::SameLine();
                ImGui::PushItemWidth(-1);
                if (ImGui::InputText("##GOName", nameBuffer, sizeof(nameBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    std::string newName = nameBuffer;
                    if (!newName.empty() && newName != selectedObject->GetName())
                    {
                        selectedObject->SetName(newName);
                        if (sceneManager)
                        {
                            if (Core::Project::HasPath())
                            {
                                // Save only the owning scene of this object
                                for (auto &nm : sceneManager->GetSceneNames())
                                {
                                    auto *sc = sceneManager->GetScene(nm);
                                    if (!sc)
                                        continue;
                                    for (auto *go : sc->GetAllGameObjects())
                                    {
                                        if (go == selectedObject)
                                        {
                                            Core::SceneSerialization::SaveSceneToFile(sc, Core::Project::GetScenesPath() + "/" + nm + ".scene");
                                            goto rename_saved;
                                        }
                                    }
                                }
                            }
                        }
                    rename_saved:;
                    }
                }
                ImGui::PopItemWidth();
                ImGui::Separator();

                auto transform = selectedObject->GetTransform();
                if (transform)
                {
                    ImGui::Text("Transform");
                    // Editable Position (X, Y, Z)
                    glm::vec3 pos = transform->GetPosition();
                    if (ImGui::DragFloat3("Position", &pos.x, 0.1f))
                    {
                        transform->SetPosition(pos);
                        if (sceneManager && Core::Project::HasPath())
                        {
                            for (auto &nm : sceneManager->GetSceneNames())
                            {
                                auto *sc = sceneManager->GetScene(nm);
                                if (!sc)
                                    continue;
                                for (auto *go : sc->GetAllGameObjects())
                                    if (go == selectedObject)
                                    {
                                        Core::SceneSerialization::SaveSceneToFile(sc, Core::Project::GetScenesPath() + "/" + nm + ".scene");
                                        goto pos_saved;
                                    }
                            }
                        }
                    pos_saved:;
                    }

                    // Editable Rotation (degrees). For 2D we mainly care about Z, but expose all 3.
                    glm::vec3 rot = transform->GetRotation();
                    if (ImGui::DragFloat3("Rotation", &rot.x, 0.5f))
                    {
                        transform->SetRotation(rot);
                        if (sceneManager && Core::Project::HasPath())
                        {
                            for (auto &nm : sceneManager->GetSceneNames())
                            {
                                auto *sc = sceneManager->GetScene(nm);
                                if (!sc)
                                    continue;
                                for (auto *go : sc->GetAllGameObjects())
                                    if (go == selectedObject)
                                    {
                                        Core::SceneSerialization::SaveSceneToFile(sc, Core::Project::GetScenesPath() + "/" + nm + ".scene");
                                        goto rot_saved;
                                    }
                            }
                        }
                    rot_saved:;
                    }

                    // Editable Scale (uniform or per-axis). Ensure scale not zero to avoid degenerate matrix.
                    glm::vec3 scl = transform->GetScale();
                    if (ImGui::DragFloat3("Scale", &scl.x, 0.05f, 0.0001f, 1000.0f))
                    {
                        for (int i = 0; i < 3; ++i)
                            if (scl[i] == 0.0f)
                                scl[i] = 0.0001f;
                        transform->SetScale(scl);
                        if (sceneManager && Core::Project::HasPath())
                        {
                            for (auto &nm : sceneManager->GetSceneNames())
                            {
                                auto *sc = sceneManager->GetScene(nm);
                                if (!sc)
                                    continue;
                                for (auto *go : sc->GetAllGameObjects())
                                    if (go == selectedObject)
                                    {
                                        Core::SceneSerialization::SaveSceneToFile(sc, Core::Project::GetScenesPath() + "/" + nm + ".scene");
                                        goto scale_saved;
                                    }
                            }
                        }
                    scale_saved:;
                    }

                    ImGui::Separator();
                }

                auto sprite = selectedObject->GetComponent<Graphics::SpriteRenderer>();
                if (sprite)
                {
                    ImGui::Separator();
                    ImGui::Text("Sprite Renderer");
                    bool enabled = sprite->IsEnabled();
                    if (ImGui::Checkbox("Enabled##SpriteRenderer", &enabled))
                    {
                        sprite->SetEnabled(enabled);
                    }
                    ImGui::Text("Has texture: %s", sprite->GetTexture() ? "Yes" : "No");
                    if (!g_animationClips.empty())
                    {
                        ImGui::Separator();
                        ImGui::Text("Animation");
                        std::vector<const char *> names;
                        names.reserve(g_animationClips.size() + 1);
                        names.push_back("<None>");
                        for (auto &c : g_animationClips)
                            names.push_back(c.name.c_str());
                        int assigned = EditorUI::GetAssignedClip(selectedObject); // -1 none
                        int current = assigned + 1;                               // shift
                        if (ImGui::Combo("Clip", &current, names.data(), (int)names.size()))
                        {
                            if (current == 0)
                                EditorUI::SetAssignedClip(selectedObject, -1);
                            else
                                EditorUI::SetAssignedClip(selectedObject, current - 1);
                        }
                        ImGui::TextDisabled("Assignments saved to animation_assignments.json");
                    }
                }

                // Camera component UI
                if (auto *cam = selectedObject->GetComponent<Core::Camera>())
                {
                    ImGui::Separator();
                    ImGui::Text("Camera");
                    bool camEnabled = cam->IsEnabled();
                    if (ImGui::Checkbox("Enabled##Camera", &camEnabled))
                    {
                        cam->SetEnabled(camEnabled);
                    }
                    float ortho = cam->GetOrthographicSize();
                    if (ImGui::DragFloat("Ortho Size", &ortho, 0.1f, 0.01f, 1000.0f))
                    {
                        cam->SetOrthographicSize(ortho);
                    }
                    float zoom = cam->GetZoom();
                    if (ImGui::DragFloat("Zoom", &zoom, 0.01f, 0.01f, 100.0f))
                    {
                        cam->SetZoom(zoom);
                    }
                    if (sceneManager)
                    {
                        // Find owning scene
                        Core::Scene *owningScene = nullptr;
                        auto names = sceneManager->GetSceneNames();
                        for (auto &nm : names)
                        {
                            auto *sc = sceneManager->GetScene(nm);
                            if (!sc)
                                continue;
                            auto objs = sc->GetAllGameObjects();
                            for (auto *go : objs)
                            {
                                if (go == selectedObject)
                                {
                                    owningScene = sc;
                                    break;
                                }
                            }
                            if (owningScene)
                                break;
                        }
                        if (owningScene)
                        {
                            bool isDesignated = (owningScene->GetDesignatedCamera() == cam);
                            bool checkbox = isDesignated;
                            if (ImGui::Checkbox("Designated Scene Camera", &checkbox))
                            {
                                if (checkbox)
                                {
                                    owningScene->SetDesignatedCamera(cam);
                                }
                                else if (isDesignated)
                                {
                                    owningScene->SetDesignatedCamera(nullptr);
                                }
                                if (Core::Project::HasPath())
                                {
                                    auto nm = sceneManager->GetSceneName(owningScene);
                                    if (!nm.empty())
                                        Core::SceneSerialization::SaveSceneToFile(owningScene, Core::Project::GetScenesPath() + "/" + nm + ".scene");
                                }
                            }
                        }
                    }
                }

                // Script component UI
                if (auto *script = selectedObject->GetComponent<Core::ScriptComponent>())
                {
                    ImGui::Separator();
                    ImGui::Text("Script");
                    std::string path = script->GetScriptPath();
                    ImGui::TextWrapped("Path: %s", path.c_str());
                    if (ImGui::Button("Open In Editor"))
                    {
                        EditorUI::OpenScriptEditor(path);
                    }
                }

                // Rigidbody2D component UI
                if (auto *rb = selectedObject->GetComponent<Core::Rigidbody2D>())
                {
                    ImGui::Separator();
                    ImGui::Text("Rigidbody 2D");
                    bool enabled = rb->IsEnabled();
                    if (ImGui::Checkbox("Enabled##Rigidbody2D", &enabled))
                        rb->SetEnabled(enabled);

                    // Body Type
                    const char *types[] = {"Static", "Kinematic", "Dynamic"};
                    int typeIndex = 2; // default Dynamic
                    auto bt = rb->GetBodyType();
                    if (bt == Core::Rigidbody2D::BodyType::Static)
                        typeIndex = 0;
                    else if (bt == Core::Rigidbody2D::BodyType::Kinematic)
                        typeIndex = 1;
                    if (ImGui::Combo("Body Type", &typeIndex, types, IM_ARRAYSIZE(types)))
                    {
                        Core::Rigidbody2D::BodyType newType = Core::Rigidbody2D::BodyType::Dynamic;
                        if (typeIndex == 0)
                            newType = Core::Rigidbody2D::BodyType::Static;
                        else if (typeIndex == 1)
                            newType = Core::Rigidbody2D::BodyType::Kinematic;
                        rb->SetBodyType(newType);
                    }

                    // Gravity options
                    bool useGrav = rb->GetUseGravity();
                    if (ImGui::Checkbox("Use Gravity", &useGrav))
                        rb->SetUseGravity(useGrav);
                    float gScale = rb->GetGravityScale();
                    if (ImGui::DragFloat("Gravity Scale", &gScale, 0.05f, -10.0f, 10.0f, "%.2f"))
                        rb->SetGravityScale(gScale);

                    // Optional: show global gravity (read-only)
                    if (auto *sc = selectedObject->GetScene())
                    {
                        auto g = sc->GetPhysics2D()->GetGravity();
                        ImGui::TextDisabled("Global Gravity: (%.2f, %.2f)", g.x, g.y);
                    }
                }
                if (auto *bc = selectedObject->GetComponent<Core::BoxCollider2D>())
                {
                    ImGui::Separator();
                    ImGui::Text("Box Collider 2D");
                    bool enabled = bc->IsEnabled();
                    if (ImGui::Checkbox("Enabled##BoxCol", &enabled))
                    {
                        bc->SetEnabled(enabled);
                        if (Core::Project::HasPath())
                        {
                            auto *owningScene = selectedObject->GetScene();
                            if (owningScene)
                            {
                                auto nm = sceneManager->GetSceneName(owningScene);
                                if (!nm.empty())
                                    Core::SceneSerialization::SaveSceneToFile(owningScene, Core::Project::GetScenesPath() + "/" + nm + ".scene");
                            }
                        }
                    }
                    bool trig = bc->IsTrigger();
                    if (ImGui::Checkbox("Is Trigger", &trig))
                    {
                        bc->SetTrigger(trig);
                        if (Core::Project::HasPath())
                        {
                            auto *owningScene = selectedObject->GetScene();
                            if (owningScene)
                            {
                                auto nm = sceneManager->GetSceneName(owningScene);
                                if (!nm.empty())
                                    Core::SceneSerialization::SaveSceneToFile(owningScene, Core::Project::GetScenesPath() + "/" + nm + ".scene");
                            }
                        }
                    }
                    glm::vec2 size = bc->GetSize();
                    if (ImGui::DragFloat2("Size", &size.x, 0.01f, 0.0001f, 10000.f))
                    {
                        bc->SetSize(size);
                        if (Core::Project::HasPath())
                        {
                            auto *owningScene = selectedObject->GetScene();
                            if (owningScene)
                            {
                                auto nm = sceneManager->GetSceneName(owningScene);
                                if (!nm.empty())
                                    Core::SceneSerialization::SaveSceneToFile(owningScene, Core::Project::GetScenesPath() + "/" + nm + ".scene");
                            }
                        }
                    }
                    glm::vec2 off = bc->GetOffset();
                    if (ImGui::DragFloat2("Offset", &off.x, 0.01f, -10000.f, 10000.f))
                    {
                        bc->SetOffset(off);
                        if (Core::Project::HasPath())
                        {
                            auto *owningScene = selectedObject->GetScene();
                            if (owningScene)
                            {
                                auto nm = sceneManager->GetSceneName(owningScene);
                                if (!nm.empty())
                                    Core::SceneSerialization::SaveSceneToFile(owningScene, Core::Project::GetScenesPath() + "/" + nm + ".scene");
                            }
                        }
                    }
                    ImGui::TextDisabled("Auto-sized from sprite if zero at Start");
                }
                if (auto *tilemap = selectedObject->GetComponent<Core::Tilemap>())
                {
                    ImGui::Separator();
                    ImGui::Text("Tilemap");
                    int w = tilemap->GetWidth();
                    int h = tilemap->GetHeight();
                    float tw = tilemap->GetTileWidth();
                    float th = tilemap->GetTileHeight();
                    int hf = tilemap->GetHFrames();
                    int vf = tilemap->GetVFrames();
                    char texBuf[512];
                    auto texPath = tilemap->GetTexturePath();
                    strncpy(texBuf, texPath.c_str(), sizeof(texBuf));
                    texBuf[sizeof(texBuf) - 1] = '\0';
                    if (ImGui::InputInt("Map Width", &w) && w > 0)
                        tilemap->SetMapSize(w, tilemap->GetHeight());
                    if (ImGui::InputInt("Map Height", &h) && h > 0)
                        tilemap->SetMapSize(tilemap->GetWidth(), h);
                    if (ImGui::InputFloat("Tile Width", &tw) && tw > 0)
                        tilemap->SetTileSize(tw, tilemap->GetTileHeight());
                    if (ImGui::InputFloat("Tile Height", &th) && th > 0)
                        tilemap->SetTileSize(tilemap->GetTileWidth(), th);
                    bool framesChanged = false;
                    if (ImGui::InputInt("H Frames", &hf))
                        framesChanged = true;
                    if (ImGui::InputInt("V Frames", &vf))
                        framesChanged = true;
                    if (framesChanged)
                    {
                        if (hf < 1)
                            hf = 1;
                        if (vf < 1)
                            vf = 1;
                        tilemap->SetTileset(tilemap->GetTexturePath(), hf, vf);
                    }
                    if (ImGui::InputText("Texture", texBuf, IM_ARRAYSIZE(texBuf)))
                    {
                        std::string np = texBuf;
                        if (!np.empty())
                            tilemap->SetTileset(np, tilemap->GetHFrames(), tilemap->GetVFrames());
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Assets"))
                    {
                        RefreshAssetList(true);
                        ImGui::OpenPopup("TilemapTexturePicker");
                    }
                    if (ImGui::BeginPopup("TilemapTexturePicker"))
                    {
                        ImGui::TextDisabled("Select Texture");
                        ImGui::Separator();
                        const auto &assets = GetAssetFiles();
                        static ImGuiTextFilter texFilter;
                        texFilter.Draw("Filter", 180);
                        ImGui::BeginChild("texpick_scroll", ImVec2(260, 200), true);
                        for (auto &p : assets)
                        {
                            if (!texFilter.PassFilter(p.c_str()))
                                continue;
                            std::filesystem::path fp(p);
                            std::string fname = fp.filename().string();
                            if (ImGui::Selectable(fname.c_str()))
                            {
                                std::string rel = p;
                                // If project path active, shorten to relative under assets
                                if (Core::Project::HasPath())
                                {
                                    auto root = Core::Project::GetAssetsPath();
                                    if (p.rfind(root, 0) == 0)
                                        rel = p.substr(root.size() + 1);
                                }
                                tilemap->SetTileset(rel, tilemap->GetHFrames(), tilemap->GetVFrames());
                                strncpy(texBuf, rel.c_str(), sizeof(texBuf));
                                texBuf[sizeof(texBuf) - 1] = '\0';
                                ImGui::CloseCurrentPopup();
                                // Save scene after change
                                if (Core::Project::HasPath())
                                {
                                    auto *owningScene = selectedObject->GetScene();
                                    if (owningScene)
                                    {
                                        auto nm = sceneManager->GetSceneName(owningScene);
                                        if (!nm.empty())
                                            Core::SceneSerialization::SaveSceneToFile(owningScene, Core::Project::GetScenesPath() + "/" + nm + ".scene");
                                    }
                                }
                            }
                            if (ImGui::IsItemHovered())
                                ImGui::SetTooltip("%s", p.c_str());
                        }
                        ImGui::EndChild();
                        ImGui::EndPopup();
                    }
                    ImGui::Separator();
                    ImGui::Text("Palette");
                    int frameCount = tilemap->GetHFrames() * tilemap->GetVFrames();
                    int cols = tilemap->GetHFrames();
                    float thumb = 32.0f;
                    if (auto tex = GetOrLoadTexture(tilemap->GetTexturePath()))
                    {
                        auto &flags = tilemap->GetColliderFlags();
                        for (int i = 0; i < frameCount; ++i)
                        {
                            int fx = i % tilemap->GetHFrames();
                            int fy = i / tilemap->GetHFrames();
                            float u0 = (float)fx / (float)tilemap->GetHFrames();
                            float v0 = (float)fy / (float)tilemap->GetVFrames();
                            float u1 = (float)(fx + 1) / (float)tilemap->GetHFrames();
                            float v1 = (float)(fy + 1) / (float)tilemap->GetVFrames();
                            if (i % cols != 0)
                                ImGui::SameLine();
                            ImGui::PushID(i);
                            ImGui::Image((void *)(intptr_t)tex->GetID(), ImVec2(thumb, thumb), ImVec2(u0, v0), ImVec2(u1, v1));
                            bool left = ImGui::IsItemClicked(ImGuiMouseButton_Left);
                            bool right = ImGui::IsItemClicked(ImGuiMouseButton_Right);
                            if (left)
                            {
                                if (g_tilemapColliderMode)
                                {
                                    if (i >= 0 && i < (int)flags.size())
                                    {
                                        flags[i] = flags[i] ? 0 : 1;
                                        tilemap->RebuildColliders();
                                    }
                                }
                                else if (g_tilemapPaintMode)
                                {
                                    g_tilemapPaintIndex = i;
                                    g_tilemapBrushErase = false;
                                }
                            }
                            if (right && g_tilemapPaintMode)
                            {
                                g_tilemapPaintIndex = i;
                                g_tilemapBrushErase = true;
                            }
                            auto *dl = ImGui::GetWindowDrawList();
                            ImVec2 p0 = ImGui::GetItemRectMin();
                            ImVec2 p1 = ImGui::GetItemRectMax();
                            if (g_tilemapPaintMode && g_tilemapPaintIndex == i)
                                dl->AddRect(p0, p1, IM_COL32(255, 255, 0, 255), 0, 0, 2.0f);
                            if (i >= 0 && i < (int)flags.size() && flags[i])
                                dl->AddRect(p0, p1, IM_COL32(255, 0, 0, 255), 0, 0, 2.0f);
                            ImGui::PopID();
                        }
                    }
                    else
                    {
                        ImGui::TextDisabled("No texture loaded");
                    }
                    ImGui::Separator();
                    if (ImGui::Checkbox("Paint Mode", &g_tilemapPaintMode))
                    {
                        if (g_tilemapPaintMode)
                            g_tilemapColliderMode = false;
                    }
                    ImGui::SameLine();
                    if (ImGui::Checkbox("Collider Mode", &g_tilemapColliderMode))
                    {
                        if (g_tilemapColliderMode)
                            g_tilemapPaintMode = false;
                    }
                    if (g_tilemapPaintMode)
                        ImGui::TextDisabled("Palette: LMB select, RMB erase brush");
                    else if (g_tilemapColliderMode)
                        ImGui::TextDisabled("Palette: LMB toggle collider (red = collidable)");
                    else
                        ImGui::TextDisabled("Enable a mode to edit");
                }
            }
            else
            {
                ImGui::Text("No object selected");
            }
        }
        ImGui::End();
    }

    void EditorUI::RenderProjectBar(Core::SceneManager *sceneManager)
    {
        constexpr float kTopBarHeight = 34.0f;
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, kTopBarHeight));
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
        if (ImGui::Begin("TopBar", nullptr, flags))
        {
            if (!Core::Project::HasPath())
            {
                ImGui::TextColored(ImVec4(1, 0.6f, 0, 1), "No project selected");
                ImGui::SameLine();
            }
            if (ImGui::Button("Open Project"))
            {
#if defined(__APPLE__)
                if (const char *dir = Kiaak_ShowOpenDirectoryDialog())
                {
                    Core::Project::SetPath(dir);
                    Core::Project::EnsureStructure();
                    // Load project-specific editor config now that path is known
                    LoadEditorConfig();
                    if (sceneManager)
                    {
                        for (auto &nm : sceneManager->GetSceneNames())
                        {
                            auto *sc = sceneManager->GetScene(nm);
                            if (sc)
                                Core::SceneSerialization::SaveSceneToFile(sc, Core::Project::GetScenesPath() + "/" + nm + ".scene");
                        }
                    }
                    RefreshAssetList(true);
                }
#else
                // TODO: other platforms
#endif
            }
            ImGui::SameLine();
            if (ImGui::Button("Create Project"))
            {
#if defined(__APPLE__)
                if (const char *dir = Kiaak_ShowOpenDirectoryDialog())
                {
                    Core::Project::SetPath(dir);
                    Core::Project::EnsureStructure();
                    // New project: save initial config so file exists, then load (redundant but safe)
                    SaveEditorConfig();
                    LoadEditorConfig();
                    if (sceneManager && sceneManager->GetSceneNames().empty())
                        sceneManager->CreateScene("MainScene");
                    // Persist current scenes
                    if (sceneManager)
                    {
                        for (auto &nm : sceneManager->GetSceneNames())
                        {
                            auto *sc = sceneManager->GetScene(nm);
                            if (sc)
                                Core::SceneSerialization::SaveSceneToFile(sc, Core::Project::GetScenesPath() + "/" + nm + ".scene");
                        }
                    }
                    RefreshAssetList(true);
                }
#else
#endif
            }

            // Texture filter mode combo (simple vertical separator via text spacing)
            ImGui::SameLine();
            ImGui::TextDisabled("|");
            ImGui::SameLine();
            ImGui::Text("Filter:");
            ImGui::SameLine();
            int currentFilter = (int)Texture::GetGlobalFilterMode();
            const char *filterLabels[] = {"Linear", "Nearest"};
            ImGui::SetNextItemWidth(90.0f);
            if (ImGui::Combo("##TexFilter", &currentFilter, filterLabels, IM_ARRAYSIZE(filterLabels)))
            {
                Texture::SetGlobalFilterMode(static_cast<Texture::FilterMode>(currentFilter));
                SaveEditorConfig();
            }
            else
            {
                // Periodic check to ensure we flush config if changed elsewhere
                SaveEditorConfig();
            }

            float windowW = ImGui::GetWindowSize().x;
            // Center custom vector play/pause button (avoid missing glyph '?')
            ImGui::SetCursorPos(ImVec2(windowW * 0.5f - 55.0f, 4.0f));
            if (auto *eng = Engine::Get())
            {
                bool editing = eng->IsEditorMode(); // editing=true means we show Play triangle
                const float btnW = 110.0f;
                const float btnH = kTopBarHeight - 8.0f;
                ImVec2 btnPos = ImGui::GetCursorScreenPos();
                ImGui::InvisibleButton("PlayPauseBtn", ImVec2(btnW, btnH));
                bool hovered = ImGui::IsItemHovered();
                bool held = ImGui::IsItemActive();
                bool clicked = ImGui::IsItemClicked();

                // Colors: vibrant green when idle (ready to play), neutral scheme while playing
                ImVec4 basePlay = ImVec4(0.14f, 0.70f, 0.25f, 1.0f);
                ImVec4 basePause = ImGui::GetStyle().Colors[ImGuiCol_Button];
                ImVec4 hovPlay = ImVec4(0.20f, 0.80f, 0.32f, 1.0f);
                ImVec4 hovPause = ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered];
                ImVec4 actPlay = ImVec4(0.09f, 0.55f, 0.18f, 1.0f);
                ImVec4 actPause = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];
                ImVec4 bg = editing ? basePlay : basePause;
                if (hovered)
                    bg = editing ? hovPlay : hovPause;
                if (held)
                    bg = editing ? actPlay : actPause;

                ImDrawList *dl = ImGui::GetWindowDrawList();
                ImVec2 brMin = btnPos;
                ImVec2 brMax = ImVec2(btnPos.x + btnW, btnPos.y + btnH);
                float rounding = 6.0f;
                dl->AddRectFilled(brMin, brMax, ImGui::GetColorU32(bg), rounding);
                dl->AddRect(brMin, brMax, ImGui::GetColorU32(ImVec4(0, 0, 0, 0.35f)), rounding);

                // Icon geometry (centered slightly upward to make room for text)
                ImVec2 center((brMin.x + brMax.x) * 0.5f, (brMin.y + brMax.y) * 0.5f - 2.0f);
                float iconH = btnH * 0.45f;
                float iconW = iconH;
                ImU32 iconCol = ImGui::GetColorU32(ImVec4(1, 1, 1, 0.95f));
                if (editing)
                {
                    // Play triangle
                    ImVec2 p0(center.x - iconW * 0.42f, center.y - iconH * 0.60f);
                    ImVec2 p1(center.x - iconW * 0.42f, center.y + iconH * 0.60f);
                    ImVec2 p2(center.x + iconW * 0.70f, center.y);
                    dl->AddTriangleFilled(p0, p1, p2, iconCol);
                }
                else
                {
                    // Pause bars
                    float barW = iconW * 0.30f;
                    float gap = iconW * 0.24f;
                    float barH = iconH * 1.15f;
                    ImVec2 leftMin(center.x - (barW + gap * 0.5f), center.y - barH * 0.5f);
                    ImVec2 leftMax(leftMin.x + barW, leftMin.y + barH);
                    ImVec2 rightMin(center.x + gap * 0.5f, center.y - barH * 0.5f);
                    ImVec2 rightMax(rightMin.x + barW, rightMin.y + barH);
                    dl->AddRectFilled(leftMin, leftMax, iconCol, 2.0f);
                    dl->AddRectFilled(rightMin, rightMax, iconCol, 2.0f);
                }

                // Descriptive text label (kept small)
                const char *txt = editing ? "Play" : "Pause";
                ImVec2 ts = ImGui::CalcTextSize(txt);
                ImVec2 tp(center.x - ts.x * 0.5f, brMax.y - ts.y - 4.0f);

                if (hovered)
                    ImGui::SetTooltip(editing ? "Start Play Mode" : "Return to Edit Mode");
                if (clicked)
                    eng->TogglePlayPause();
            }
        }
        ImGui::End();
    }

    void EditorUI::RenderAssetBrowser()
    {
        // Left column bottom panel (same width as hierarchy: 320)
        float hierarchyBottomY = ImGui::GetIO().DisplaySize.y - 200; // current hard-coded bottom row height 200
        constexpr float leftWidth = 320.0f;
        ImGui::SetNextWindowPos(ImVec2(0, hierarchyBottomY));
        ImGui::SetNextWindowSize(ImVec2(leftWidth, 200));
        if (ImGui::Begin("Asset Browser", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
        {
            if (!Core::Project::HasPath())
            {
                ImGui::TextWrapped("No project selected. Use Open/Create Project to choose a folder.");
                ImGui::End();
                return;
            }

            static char importSource[512] = ""; // absolute or relative path to existing file
            static char importName[256] = "";   // optional target filename
            static std::string importStatus;
            bool doRefresh = false;

            // Manual refresh button
            if (ImGui::Button("Refresh"))
            {
                RefreshAssetList(true);
            }

            ImGui::SameLine();
            if (ImGui::Button("Import Asset"))
            {
                ImGui::OpenPopup("ImportAssetPopup");
                importStatus.clear();
            }
            if (ImGui::BeginPopupModal("ImportAssetPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("Select a file to copy into assets/.");
                ImGui::InputText("Source Path", importSource, sizeof(importSource));
                ImGui::InputText("Target Name (optional)", importName, sizeof(importName));
                ImGui::SameLine();
                if (ImGui::Button("Browse"))
                {
#if defined(__APPLE__)
                    if (const char *chosen = Kiaak_ShowOpenFileDialog())
                    {
                        strncpy(importSource, chosen, sizeof(importSource) - 1);
                        importSource[sizeof(importSource) - 1] = '\0';
                        if (strlen(importName) == 0)
                        {
                            // prefill target name with filename
                            std::filesystem::path p(chosen);
                            auto fname = p.filename().string();
                            strncpy(importName, fname.c_str(), sizeof(importName) - 1);
                            importName[sizeof(importName) - 1] = '\0';
                        }
                    }
#else
                    importStatus = "Native file dialog not implemented on this platform yet";
#endif
                }
                ImGui::TextDisabled("Supported: .png .jpg .jpeg .bmp .tga");
                if (!importStatus.empty())
                {
                    ImGui::Separator();
                    ImGui::TextWrapped("%s", importStatus.c_str());
                }
                if (ImGui::Button("Import"))
                {
                    namespace fs = std::filesystem;
                    try
                    {
                        fs::path src(importSource);
                        if (!fs::exists(src) || !fs::is_regular_file(src))
                        {
                            importStatus = "Source file not found";
                        }
                        else
                        {
                            auto ext = src.extension().string();
                            for (auto &c : ext)
                                c = (char)tolower(c);
                            if (ext != ".png" && ext != ".jpg" && ext != ".jpeg" && ext != ".bmp" && ext != ".tga")
                            {
                                importStatus = "Unsupported extension";
                            }
                            else
                            {
                                fs::path targetDir(kAssetDir);
                                if (Core::Project::HasPath())
                                    targetDir = Core::Project::GetAssetsPath();
                                if (!fs::exists(targetDir))
                                    fs::create_directories(targetDir);
                                fs::path targetName = (strlen(importName) > 0) ? fs::path(importName) : src.filename();
                                if (targetName.extension().empty())
                                    targetName += src.extension();
                                fs::path target = targetDir / targetName;
                                fs::copy_file(src, target, fs::copy_options::overwrite_existing);
                                importStatus = std::string("Imported -> ") + target.generic_string();
                                doRefresh = true;
                            }
                        }
                    }
                    catch (const std::exception &e)
                    {
                        importStatus = std::string("Error: ") + e.what();
                    }
                    catch (...)
                    {
                        importStatus = "Unknown error";
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Close"))
                {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
            if (doRefresh)
                RefreshAssetList(true);

            RefreshAssetList();
            const auto &assets = GetAssetFiles();
            ImGui::Separator();
            ImGui::BeginChild("AssetList");
            int assetIndex = 0;
            for (auto &a : assets)
            {
                std::filesystem::path p(a);
                std::string fname = p.filename().string();
                ImGui::PushID(assetIndex++);
                if (ImGui::Selectable(fname.c_str()))
                {
                    // Handle asset selection (if future logic added)
                }
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("%s", a.c_str());
                ImGui::PopID();
            }
            ImGui::EndChild();
        }
        ImGui::End();
    }

    void EditorUI::RenderAnimatorPanel()
    {
        // Bottom row to the right of asset browser fills remaining width
        float hierarchyBottomY = ImGui::GetIO().DisplaySize.y - 200; // same 200px height row
        constexpr float leftWidth = 320.0f;
        float remainingWidth = ImGui::GetIO().DisplaySize.x - leftWidth;
        if (remainingWidth < 10)
            remainingWidth = 10;
        ImGui::SetNextWindowPos(ImVec2(leftWidth, hierarchyBottomY));
        ImGui::SetNextWindowSize(ImVec2(remainingWidth, 200));
        if (ImGui::Begin("Animator", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
        {
            ImGui::Text("Animation Clips");
            ImGui::Separator();
            if (ImGui::Button("Save Clips"))
            {
                std::ofstream out(kAnimationClipsFile);
                if (out.good())
                    WriteClipsJSON(g_animationClips, out);
            }
            // List clips
            for (int i = 0; i < (int)g_animationClips.size(); ++i)
            {
                auto &c = g_animationClips[i];
                ImGui::PushID(i);
                bool selected = (i == g_selectedClipIndex);
                if (ImGui::Selectable(c.name.c_str(), selected))
                    g_selectedClipIndex = i;
                if (ImGui::BeginPopupContextItem("ClipCtx"))
                {
                    if (ImGui::MenuItem("Edit Sheet"))
                    {
                        g_sheetEditorClipIndex = i;
                        g_openSheetEditor = true;
                    }
                    if (ImGui::MenuItem("Delete"))
                    {
                        g_animationClips.erase(g_animationClips.begin() + i);
                        if (g_selectedClipIndex >= (int)g_animationClips.size())
                            g_selectedClipIndex = (int)g_animationClips.size() - 1;
                        ImGui::EndPopup();
                        ImGui::PopID();
                        break;
                    }
                    ImGui::EndPopup();
                }
                ImGui::PopID();
            }
            if (ImGui::Button("New Clip"))
            {
                AnimationClipInfo clip;
                clip.name = "Anim" + std::to_string((int)g_animationClips.size() + 1);
                g_animationClips.push_back(clip);
            }
            ImGui::SameLine();
            if (ImGui::Button("Edit"))
            {
                if (g_selectedClipIndex >= 0)
                {
                    g_sheetEditorClipIndex = g_selectedClipIndex;
                    g_openSheetEditor = true;
                }
            }
            // Settings for selected clip (rename / fps / autoplay + runtime control)
            if (g_selectedClipIndex >= 0 && g_selectedClipIndex < (int)g_animationClips.size())
            {
                auto &cset = g_animationClips[g_selectedClipIndex];
                ImGui::Separator();
                ImGui::Text("Clip Settings");
                // Inline rename
                static char clipNameBuf[256];
                // Sync buffer if different and no active edit
                if (!ImGui::IsAnyItemActive())
                {
                    if (strncmp(clipNameBuf, cset.name.c_str(), sizeof(clipNameBuf)) != 0)
                    {
                        strncpy(clipNameBuf, cset.name.c_str(), sizeof(clipNameBuf) - 1);
                        clipNameBuf[sizeof(clipNameBuf) - 1] = '\0';
                    }
                }
                ImGui::Text("Name");
                ImGui::SameLine();
                ImGui::PushItemWidth(200.0f);
                bool nameChanged = false;
                if (ImGui::InputText("##ClipNameEdit", clipNameBuf, sizeof(clipNameBuf), ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    nameChanged = true;
                }
                // Also commit when focus leaves the item after editing
                if (ImGui::IsItemDeactivatedAfterEdit())
                    nameChanged = true;
                ImGui::PopItemWidth();
                if (nameChanged)
                {
                    std::string newName = clipNameBuf;
                    if (!newName.empty() && newName != cset.name)
                    {
                        cset.name = newName;
                        // Auto-save all clips immediately after rename
                        std::ofstream out(kAnimationClipsFile);
                        if (out.good())
                            WriteClipsJSON(g_animationClips, out);
                    }
                }
                ImGui::SliderFloat("FPS", &cset.fps, 1.0f, 60.0f, "%.1f");
                ImGui::Checkbox("Auto Play", &cset.autoPlay);
                if (ImGui::Button("Play"))
                {
                    for (auto &pair : g_objectClipAssignments)
                    {
                        if (pair.second == g_selectedClipIndex && pair.first)
                        {
                            if (auto *anim = pair.first->GetComponent<Core::Animator>())
                            {
                                anim->SetClipIndex(g_selectedClipIndex);
                                anim->Play();
                            }
                        }
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Stop"))
                {
                    for (auto &pair : g_objectClipAssignments)
                    {
                        if (pair.second == g_selectedClipIndex && pair.first)
                        {
                            if (auto *anim = pair.first->GetComponent<Core::Animator>())
                                anim->Stop();
                        }
                    }
                }
            }
            // Selected clip frame thumbnails
            if (g_selectedClipIndex >= 0 && g_selectedClipIndex < (int)g_animationClips.size())
            {
                auto &clip = g_animationClips[g_selectedClipIndex];
                if (!clip.texturePath.empty() && clip.hFrames > 0 && clip.vFrames > 0 && !clip.sequence.empty())
                {
                    if (auto tex = GetOrLoadTexture(clip.texturePath))
                    {
                        ImGui::Separator();
                        ImGui::Text("Frames (%zu)", clip.sequence.size());
                        float thumb = 36.0f;
                        int colsPerRow = (int)(ImGui::GetContentRegionAvail().x / (thumb + 4.0f));
                        if (colsPerRow < 1)
                            colsPerRow = 1;
                        int col = 0;
                        for (size_t i = 0; i < clip.sequence.size(); ++i)
                        {
                            int logical = clip.sequence[i];
                            int cols = clip.hFrames;
                            int r = logical / cols; // bottom = 0
                            int c = logical % cols;
                            float u0 = (float)c / (float)cols;
                            float u1 = (float)(c + 1) / (float)cols;
                            float v0 = (float)r / (float)clip.vFrames;
                            float v1 = (float)(r + 1) / (float)clip.vFrames;
                            // Show upright (not flipped)
                            ImGui::PushID((int)i);
                            ImGui::Image((void *)(intptr_t)tex->GetID(), ImVec2(thumb, thumb), ImVec2(u0, v0), ImVec2(u1, v1));
                            if (ImGui::IsItemHovered())
                            {
                                ImGui::BeginTooltip();
                                ImGui::Text("Seq %zu -> frame %d", i, logical);
                                ImGui::EndTooltip();
                            }
                            ImGui::PopID();
                            if (++col < colsPerRow)
                                ImGui::SameLine();
                            else
                                col = 0;
                        }
                    }
                }
            }
            ImGui::Separator();
            ImGui::TextDisabled("Right-click hierarchy -> New Animation for quick create");
        }
        ImGui::End();
        // Render sheet editor modal if active
        EditorUI::RenderAnimationSheetEditor();
    }

    const std::vector<EditorUI::AnimationClipInfo> &EditorUI::GetAnimationClips() { return g_animationClips; }
    int EditorUI::GetAssignedClip(Core::GameObject *go)
    {
        auto it = g_objectClipAssignments.find(go);
        return it == g_objectClipAssignments.end() ? -1 : it->second;
    }
    void EditorUI::SetAssignedClip(Core::GameObject *go, int clipIndex)
    {
        if (!go)
            return;
        if (clipIndex < 0)
            g_objectClipAssignments.erase(go);
        else
            g_objectClipAssignments[go] = clipIndex;
    }

    // Helper to draw checker background
    static void DrawChecker(ImDrawList *dl, ImVec2 pos, ImVec2 size, float cell = 8.0f)
    {
        ImU32 col1 = IM_COL32(60, 60, 60, 255);
        ImU32 col2 = IM_COL32(80, 80, 80, 255);
        for (float y = 0; y < size.y; y += cell)
        {
            for (float x = 0; x < size.x; x += cell)
            {
                bool alt = ((int)(x / cell) + (int)(y / cell)) & 1;
                dl->AddRectFilled(ImVec2(pos.x + x, pos.y + y), ImVec2(pos.x + x + cell, pos.y + y + cell), alt ? col1 : col2);
            }
        }
    }

    void EditorUI::RenderAnimationSheetEditor()
    {
        if (!g_openSheetEditor || g_sheetEditorClipIndex < 0 || g_sheetEditorClipIndex >= (int)g_animationClips.size())
            return;
        auto &clip = g_animationClips[g_sheetEditorClipIndex];
        ImGui::SetNextWindowSize(ImVec2(900, 600), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Animation Sheet Editor", &g_openSheetEditor, ImGuiWindowFlags_NoCollapse))
        {
            ImGui::Text("Editing: %s", clip.name.c_str());
            ImGui::Separator();
            // Left panel: texture & grid
            ImGui::BeginChild("SheetLeft", ImVec2(ImGui::GetContentRegionAvail().x * 0.65f, 0), true);
            if (clip.texturePath.empty())
            {
                ImGui::Text("Select Sprite Sheet (double-click from assets list below)");
                RefreshAssetList();
                ImGui::Separator();
                ImGui::BeginChild("SheetAssetPick", ImVec2(0, 0), true);
                int pickIndex = 0;
                for (auto &a : GetAssetFiles())
                {
                    std::filesystem::path p(a);
                    std::string fname = p.filename().string();
                    ImGui::PushID(pickIndex++);
                    if (ImGui::Selectable(fname.c_str()))
                    {
                        clip.texturePath = a; // store full path internally
                    }
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("%s", a.c_str());
                    ImGui::PopID();
                }
                ImGui::EndChild();
            }
            else
            {
                // Display only filename but tooltip full path
                std::filesystem::path p(clip.texturePath);
                auto fname = p.filename().string();
                ImGui::TextWrapped("Sheet: %s", fname.c_str());
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("%s", clip.texturePath.c_str());
                auto tex = GetOrLoadTexture(clip.texturePath);
                if (!tex)
                {
                    ImGui::TextColored(ImVec4(1, 0.3f, 0, 1), "Failed to load texture");
                }
                else
                {
                    ImGui::Separator();
                    ImVec2 avail = ImGui::GetContentRegionAvail();
                    float areaH = avail.y - 10.0f;
                    if (areaH < 120)
                        areaH = 120;
                    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
                    ImVec2 canvasSize(avail.x, areaH);
                    auto *dl = ImGui::GetWindowDrawList();
                    dl->AddRectFilled(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), IM_COL32(30, 30, 30, 255));
                    DrawChecker(dl, canvasPos, canvasSize, 12.0f);
                    float texW = (float)tex->GetWidth();
                    float texH = (float)tex->GetHeight();
                    float scale = std::min(canvasSize.x / texW, canvasSize.y / texH);
                    ImVec2 imgSize(texW * scale, texH * scale);
                    ImVec2 imgPos(canvasPos.x + (canvasSize.x - imgSize.x) * 0.5f, canvasPos.y + (canvasSize.y - imgSize.y) * 0.5f);
                    ImGui::SetCursorScreenPos(imgPos);
                    // Flip vertically for correct orientation
                    ImGui::Image((void *)(intptr_t)tex->GetID(), imgSize, ImVec2(0, 1), ImVec2(1, 0));
                    int cols = clip.hFrames;
                    int rows = clip.vFrames;
                    if (cols > 0 && rows > 0)
                    {
                        float cw = imgSize.x / cols;
                        float ch = imgSize.y / rows;
                        // Input capture over canvas
                        ImGui::SetCursorScreenPos(canvasPos);
                        ImGui::InvisibleButton("SheetCanvas", canvasSize, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
                        bool hovered = ImGui::IsItemHovered();
                        if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                        {
                            ImVec2 mp = ImGui::GetIO().MousePos;
                            if (mp.x >= imgPos.x && mp.x < imgPos.x + imgSize.x && mp.y >= imgPos.y && mp.y < imgPos.y + imgSize.y)
                            {
                                int c = (int)((mp.x - imgPos.x) / cw);
                                int rVis = (int)((mp.y - imgPos.y) / ch); // 0 at top visually
                                int r = rows - 1 - rVis;                  // map to data row (bottom = 0)
                                int idx = r * cols + c;
                                if (std::find(g_tempSelection.begin(), g_tempSelection.end(), idx) == g_tempSelection.end())
                                    g_tempSelection.push_back(idx);
                            }
                        }
                        if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                            g_tempSelection.clear();
                        // Grid lines
                        ImU32 gridCol = IM_COL32(200, 200, 200, 90);
                        for (int c = 1; c < cols; ++c)
                            dl->AddLine(ImVec2(imgPos.x + c * cw, imgPos.y), ImVec2(imgPos.x + c * cw, imgPos.y + imgSize.y), gridCol, 1.0f);
                        for (int rLine = 1; rLine < rows; ++rLine)
                            dl->AddLine(ImVec2(imgPos.x, imgPos.y + rLine * ch), ImVec2(imgPos.x + imgSize.x, imgPos.y + rLine * ch), gridCol, 1.0f);
                        // Selections overlay
                        ImU32 selCol = IM_COL32(255, 180, 50, 120);
                        ImU32 selBorder = IM_COL32(255, 140, 0, 255);
                        for (size_t si = 0; si < g_tempSelection.size(); ++si)
                        {
                            int idx = g_tempSelection[si];
                            if (idx < 0)
                                continue;
                            int r = idx / cols;
                            int c = idx % cols;
                            if (r >= rows || c >= cols)
                                continue;
                            int rVis = rows - 1 - r;
                            ImVec2 a(imgPos.x + c * cw, imgPos.y + rVis * ch);
                            ImVec2 b(a.x + cw, a.y + ch);
                            dl->AddRectFilled(a, b, selCol);
                            dl->AddRect(a, b, selBorder, 0, 0, 2.0f);
                            char buf[16];
                            snprintf(buf, sizeof(buf), "%d", (int)si);
                            dl->AddText(ImVec2(a.x + 4, a.y + 4), IM_COL32(20, 20, 20, 255), buf);
                        }
                    }
                    else
                    {
                        ImGui::TextDisabled("Set H/V Frames to show grid.");
                    }
                }
            }
            ImGui::EndChild();
            ImGui::SameLine();
            // Right panel controls
            ImGui::BeginChild("SheetRight", ImVec2(0, 0), true);
            ImGui::Text("Grid Settings");
            int h = clip.hFrames;
            int v = clip.vFrames;
            if (ImGui::InputInt("H Frames", &h))
                clip.hFrames = h > 0 ? h : 0;
            if (ImGui::InputInt("V Frames", &v))
                clip.vFrames = v > 0 ? v : 0;
            if (auto tex = GetOrLoadTexture(clip.texturePath))
            {
                clip.cellWidth = (clip.hFrames > 0) ? tex->GetWidth() / clip.hFrames : 0;
                clip.cellHeight = (clip.vFrames > 0) ? tex->GetHeight() / clip.vFrames : 0;
                ImGui::Text("Cell Size: %d x %d", clip.cellWidth, clip.cellHeight);
            }
            ImGui::Separator();
            if (ImGui::Button("Clear Selection"))
                g_tempSelection.clear();
            ImGui::SameLine();
            if (ImGui::Button("Use Selection"))
            {
                clip.sequence = g_tempSelection;
                clip.dirty = true;
            }
            if (ImGui::Button("Auto Sequence"))
            {
                clip.sequence.clear();
                for (int i = 0; i < clip.hFrames * clip.vFrames; ++i)
                    clip.sequence.push_back(i);
                clip.dirty = true;
                g_tempSelection = clip.sequence;
            }
            ImGui::Separator();
            if (!clip.sequence.empty())
            {
                ImGui::Text("Sequence (%zu frames):", clip.sequence.size());
                ImGui::BeginChild("SeqList", ImVec2(0, 120), true);
                for (size_t i = 0; i < clip.sequence.size(); ++i)
                    ImGui::Text("%zu: %d", i, clip.sequence[i]);
                ImGui::EndChild();
            }
            else
                ImGui::TextDisabled("No sequence defined.");
            ImGui::Separator();
            if (ImGui::Button("Close"))
            {
                g_openSheetEditor = false;
                g_sheetEditorClipIndex = -1;
            }
            ImGui::EndChild();
        }
        ImGui::End();
    }

}