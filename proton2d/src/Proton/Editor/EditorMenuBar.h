#pragma once
#ifdef PT_EDITOR

namespace proton {

	class EditorMenuBar
	{
	public:
		void OnImGuiRender();

		void NewScene();
		void OpenScene();
		void SaveScene();
		void SaveSceneAs();

	};

}

#endif // PT_EDITOR
