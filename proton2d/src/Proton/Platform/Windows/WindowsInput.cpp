#include "ptpch.h"
#include "Proton/Platform/Windows/WindowsInput.h"
#include "Proton/Core/Application.h"

#include <GLFW/glfw3.h>

namespace proton {

    bool WindowsInput::Impl_IsKeyPressed(int keyCode)
    {
        auto window = (GLFWwindow*)Application::Get().GetWindow().GetNativeWindow();
        auto state = glfwGetKey(window, keyCode);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    glm::vec2 WindowsInput::Impl_GetMousePosition()
    {
        auto window = (GLFWwindow*)Application::Get().GetWindow().GetNativeWindow();
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        return { (float)x, (float)y };
    }
}
