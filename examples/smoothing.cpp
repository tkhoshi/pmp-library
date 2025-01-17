// Copyright 2011-2019 the Polygon Mesh Processing Library developers.
// Distributed under a MIT-style license, see LICENSE.txt for details.

#include <pmp/visualization/mesh_viewer.h>
#include <pmp/algorithms/curvature.h>
#include <pmp/algorithms/smoothing.h>
#include <imgui.h>

using namespace pmp;

class Viewer : public MeshViewer
{
public:
    Viewer(const char* title, int width, int height);

protected:
    void process_imgui() override;
};

Viewer::Viewer(const char* title, int width, int height)
    : MeshViewer(title, width, height)
{
    crease_angle_ = 180.0;
}

void Viewer::process_imgui()
{
    MeshViewer::process_imgui();

    ImGui::Spacing();
    ImGui::Spacing();

    if (ImGui::CollapsingHeader("Curvature", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::Button("Mean Curvature"))
        {
            curvature(mesh_, Curvature::mean, 1, true, true);
            curvature_to_texture_coordinates(mesh_);
            update_mesh();
            renderer_.use_cold_warm_texture();
            set_draw_mode("Texture");
        }
    }

    ImGui::Spacing();
    ImGui::Spacing();

    if (ImGui::CollapsingHeader("Smoothing", ImGuiTreeNodeFlags_DefaultOpen))
    {
        static int weight = 0;
        ImGui::RadioButton("Cotan Laplace", &weight, 0);
        ImGui::RadioButton("Uniform Laplace", &weight, 1);
        bool uniform_laplace = (weight == 1);

        static int iterations = 10;
        ImGui::PushItemWidth(100);
        ImGui::SliderInt("Iterations", &iterations, 1, 100);
        ImGui::PopItemWidth();

        if (ImGui::Button("Explicit Smoothing"))
        {
            explicit_smoothing(mesh_, iterations, uniform_laplace);
            update_mesh();
        }

        ImGui::Spacing();
        ImGui::Spacing();

        static float timestep = 0.001;
        float lb = uniform_laplace ? 1.0 : 0.001;
        float ub = uniform_laplace ? 100.0 : 1.0;
        ImGui::PushItemWidth(100);
        ImGui::SliderFloat("TimeStep", &timestep, lb, ub);
        ImGui::PopItemWidth();

        if (ImGui::Button("Implicit Smoothing"))
        {
            // does the mesh have a boundary?
            bool has_boundary = false;
            for (auto v : mesh_.vertices())
                if (mesh_.is_boundary(v))
                    has_boundary = true;

            // only re-scale if we don't have a (fixed) boundary
            bool rescale = !has_boundary;

            Scalar dt =
                uniform_laplace ? timestep : timestep * radius_ * radius_;
            try
            {
                implicit_smoothing(mesh_, dt, uniform_laplace, rescale);
            }
            catch (const SolverException& e)
            {
                std::cerr << e.what() << std::endl;
                return;
            }
            update_mesh();
        }
    }
}

int main(int argc, char** argv)
{
#ifndef __EMSCRIPTEN__
    Viewer window("Smoothing", 800, 600);
    if (argc == 2)
        window.load_mesh(argv[1]);
    return window.run();
#else
    Viewer window("Smoothing", 800, 600);
    window.load_mesh(argc == 2 ? argv[1] : "input.off");
    return window.run();
#endif
}
