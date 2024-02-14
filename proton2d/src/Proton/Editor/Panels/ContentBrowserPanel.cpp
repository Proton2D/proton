#include "ptpch.h"
#ifdef PT_EDITOR
#include "Proton/Editor/Panels/ContentBrowserPanel.h"
#include "Proton/Editor/EditorLayer.h"
#include "Proton/Scene/PrefabManager.h"
#include "Proton/Core/Timer.h"

#include <imgui.h>
#include <filesystem>

namespace proton {

	static std::filesystem::path stripFirstDir(const std::filesystem::path& path)
	{
		std::filesystem::path result;
		int i = 0;
		for (const auto& part : path)
		{
			if (i)
				result /= part;
			i++;
		}
		return result;
	}

	void ContentBrowserPanel::OnCreate()
	{
		m_BackIcon = MakeUnique<Texture>("editor/content/textures/icon/back.png");
		m_FolderIcon = MakeUnique<Texture>("editor/content/textures/icon/folder.png");
		m_FileIcon = MakeUnique<Texture>("editor/content/textures/icon/file.png");
		m_ImageIcon = MakeUnique<Texture>("editor/content/textures/icon/image.png");
		m_PrefabIcon = MakeUnique<Texture>("editor/content/textures/icon/prefab.png");
		m_SceneIcon = MakeUnique<Texture>("editor/content/textures/icon/scene.png");
	}

	static void DrawChevronRightIcon()
	{
		ImGui::SameLine();
		ImGui::PushFont(EditorLayer::GetFontAwesome());
		ImGui::Text(u8"\uF054");
		ImGui::PopFont();
		ImGui::SameLine();
	}

	void ContentBrowserPanel::OnImGuiRender()
	{
		ImGui::Begin("Content");

		DrawChevronRightIcon();

		std::filesystem::path path;
		for (const auto& part : m_CurrentDirectory) 
		{
			path /= part;
			ImGui::Text("%s", path.filename().string().c_str());
			if (ImGui::IsItemHovered())
			{
				ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
			}
			if (ImGui::IsItemClicked())
			{
				m_CurrentDirectory = path;
				Update();
				break;
			}
			DrawChevronRightIcon();
		}
		
		ImGui::Dummy({ 0, 0 }); 
		ImGui::Dummy({ 0, 3 });
		ImGui::Separator();
		ImGui::Dummy({ 0, 3 });

		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / (m_IconSize + m_Padding));
		if (columnCount)
			ImGui::Columns(columnCount, 0, false);

		if (m_CurrentDirectory != m_ContentRoot)
		{
			if (DrawDirectoryItem("..", m_BackIcon.get()))
			{
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
				Update();
			}
		}

		for (auto& folder : m_Folders)
		{
			if (DrawDirectoryItem(folder, m_FolderIcon.get()))
			{
				m_CurrentDirectory /= folder.filename();
				Update();
			}
		}

		for (auto& file : m_Files)
		{
			Texture* icon = m_FileIcon.get();
			std::string extension = file.extension().string();
			if (extension == ".json")
			{
				std::string protonExtension = file.stem().extension().string();
				if (protonExtension == ".prefab")
					icon = m_PrefabIcon.get();
				else if (protonExtension == ".scene")
					icon = m_SceneIcon.get();
			}
			else if (extension == ".png" || extension == ".jpg")
				icon = m_ImageIcon.get();

			bool clicked = false;
			clicked = DrawDirectoryItem(file, icon);
		}

		ImGui::Columns(1);
		ImGui::End();
	}

	void ContentBrowserPanel::OnUpdate(float ts)
	{
		if (m_UpdateTimer <= 0.0f)
		{
			m_Folders.clear();
			m_Files.clear();
			for (auto& entry : std::filesystem::directory_iterator(m_CurrentDirectory))
			{
				auto path = std::filesystem::relative(entry.path(), m_ContentRoot);
				if (entry.is_regular_file())
				{
					m_Files.push_back(path);
					continue;
				}
				m_Folders.push_back(path);
			}
			m_UpdateTimer = 2.0f;
		}
		m_UpdateTimer -= ts;
	}

	void ContentBrowserPanel::Update()
	{
		m_UpdateTimer = 0.0f;
	}

	bool ContentBrowserPanel::DrawDirectoryItem(const std::filesystem::path& path, Texture* icon)
	{
		std::string filename = path.filename().string();

		// Item icon
		ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.2f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.25f, 0.25f, 0.25f, 1.0f });

		ImTextureID textureID = (ImTextureID)(uint64_t)(icon->GetOpenGL_ID());
		ImGui::PushID(filename.c_str());
		ImGui::ImageButton(textureID, { m_IconSize, m_IconSize }, { 0, 1 }, { 1, 0 });
		
		if (icon == m_PrefabIcon.get() || icon == m_SceneIcon.get())
		{
			if (ImGui::BeginDragDropSource())
			{
				std::string type;
				if (icon == m_PrefabIcon.get())
					type = "CONTENT_BROWSER_PREFAB";
				else if (icon == m_SceneIcon.get())
					type = "CONTENT_BROWSER_SCENE";

				std::filesystem::path relative = stripFirstDir(path);
				const wchar_t* itemPath = relative.c_str();
				ImGui::SetDragDropPayload(type.c_str(), itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
				ImGui::EndDragDropSource();
			}
		}


		if (icon == m_BackIcon.get())
			ImGui::SetItemTooltip(m_CurrentDirectory.parent_path().filename().string().c_str());
		else
			ImGui::SetItemTooltip(filename.c_str());

		ImGui::PopStyleColor(3);
		ImGui::PopID();

		bool clicked = ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0);

		// Item filename
		ImGui::Dummy({ 0, 1 });
		ImGui::PushFont(EditorLayer::GetSmallFont());
		float textWidth = ImGui::CalcTextSize(filename.c_str()).x;
		if (textWidth < m_IconSize)
		{
			float x = ImGui::GetCursorPos().x;
			float width = ImGui::GetContentRegionAvail().x;
			ImGui::SetCursorPosX(x + (width - textWidth) * 0.5f);
		}
		ImGui::TextWrapped(filename.c_str());
		ImGui::PopFont();
		ImGui::NextColumn();

		return clicked;
	}

}

#endif // PT_EDITOR
