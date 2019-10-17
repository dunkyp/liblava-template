// file      : src/main.cpp
// copyright : Copyright (c) 2018-present, Lava Block OÜ
// license   : MIT; see accompanying LICENSE file

#include <liblava/lava.hpp>

#include <imgui.h>

using namespace lava;

int main(int argc, char* argv[]) {

    frame_config config;
    config.app = "template";
    config.cmd_line = { argc, argv };

    frame frame(config);
    if (!frame.ready())
        return error::not_ready;

    window window(config.app);
    if (!window.create())
        return error::create_failed;

    input input;
    window.assign(&input);

    auto device = frame.create_device();
    if (!device)
        return error::create_failed;

    auto render_target = create_target(&window, device);
    if (!render_target)
        return error::create_failed;

    auto frame_count = render_target->get_frame_count();

    forward_shading forward_shading;
    if (!forward_shading.create(render_target))
        return error::create_failed;

    auto render_pass = forward_shading.get_render_pass();

    gui gui;
    gui.setup(window.get());
    if (!gui.initialize(graphics_pipeline::make(device), frame_count))
        return error::create_failed;

    {
        auto gui_pipeline = gui.get_pipeline();

        gui_pipeline->set_render_pass(render_pass->get());
        if (!gui_pipeline->create())
            return error::create_failed;

        render_pass->get_subpass()->add(gui_pipeline);
    }

    auto fonts = texture::make();
    if (!gui.upload_fonts(fonts))
        return error::create_failed;

    staging staging;
    staging.add(fonts);

    block block;
    if (!block.create(device, frame_count, device->get_graphics_queue().family))
        return false;

    block.add_command([&](VkCommandBuffer cmd_buf) {

        staging.stage(cmd_buf, block.get_current_frame());

        render_pass->process(cmd_buf, block.get_current_frame());
    });

    gui.on_draw = [&]() {

        ImGui::ShowDemoWindow();
    };

    input.add(&gui);

    input.key.listeners.add([&](key_event::ref event) {

        if (event.key == key::tab && event.action == action::press)
            gui.toggle();

        if (event.key == key::escape && event.action == action::press)
            frame.shut_down();
    });

    renderer simple_renderer;
    if (!simple_renderer.create(render_target->get_swapchain()))
        return error::create_failed;

    frame.add_run([&]() {

        input.handle_events();

        if (window.has_close_request())
            return frame.shut_down();

        if (window.has_resize_request())
            return window.handle_resize();

        if (window.iconified()) {

            frame.set_wait_for_events(true);
            return true;

        } else {

            if (frame.waiting_for_events())
                frame.set_wait_for_events(false);
        }

        auto frame_index = simple_renderer.begin_frame();
        if (!frame_index)
            return true;

        if (!block.process(*frame_index))
            return false;

        return simple_renderer.end_frame(block.get_buffers());
    });

    frame.add_run_end([&]() {

        input.remove(&gui);
        gui.shutdown();

        fonts->destroy();

        block.destroy();
        forward_shading.destroy();

        simple_renderer.destroy();
        render_target->destroy();
    });

    return frame.run() ? 0 : error::aborted;
}
