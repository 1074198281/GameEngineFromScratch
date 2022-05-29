#include "PipelineState.hpp"

namespace My {
    struct RenderPipeline {
        PipelineState	state;


        void reflectMembers() {
            PipelineState	state;
            ImGui::Text("state");
            state.reflectMembers();

        }

        void reflectUI() {
            ImGui::Begin("RenderPipeline");
            reflectMembers();
            ImGui::End();
        }
    };
} // namespace My
