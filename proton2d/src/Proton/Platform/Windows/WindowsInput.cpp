#include "ptpch.h"
#include "Proton/Core/Input.h"
#include "Proton/Core/Application.h"

#include <GLFW/glfw3.h>

namespace proton {

    bool Input::IsKeyPressed(int keyCode)
    {
        auto window = (GLFWwindow*)Application::Get().GetWindow().GetNativeWindow();
        auto state = glfwGetKey(window, keyCode);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool Input::IsMouseButtonPressed(const MouseCode button)
    {
        auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        auto state = glfwGetMouseButton(window, static_cast<int32_t>(button));
        return state == GLFW_PRESS;
    }

    glm::vec2 Input::GetMousePosition()
    {
        auto window = (GLFWwindow*)Application::Get().GetWindow().GetNativeWindow();
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        return { (float)x, (float)y };
    }
}
