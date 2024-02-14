#pragma once
#ifdef PT_EDITOR
#include "Proton/Editor/Panels/EditorPanel.h"

namespace proton {

	class SceneHierarchyPanel : public EditorPanel 
	{
	public:
		virtual void OnImGuiRender() override;
	
	private:
		void DrawEntityTreeNode(Entity entity);

	private:
		Entity m_TreeNodeHovered;
		Entity m_EntityDragTarget;
	};

}
#endif // PT_EDITOR
