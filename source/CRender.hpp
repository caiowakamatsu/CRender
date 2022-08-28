// // Created by Yohwllo
// // 2022/01/17

// namespace cr
// {
//     class CRender
//     {
//     private:
//         auto logger;
//         auto render_target_options;
//         auto post_processing_options;
//         auto display;
//         auto configuration;
//         auto settings;
//         auto triangular_scenes auto scenes auto frame;
//         auto normal_frame;
//         auto albedo_frame;
//         auto sample_count;
//         auto intersect_scenes;
//         auto tasks;
//         auto mouse_pos_initialized;
//         auto previous_mouse_pos;
//         // protected:
//     public:
//         CRender()
//         {
//             logger = cr::logger();
//             render_target_options = cr::component::render_target::Options();
//             post_processing_options = cr::component::post_processing::Options();
//             display = cr::display(1920, 1080, &logger);
//             configuration =
//                 cr::scene_configuration(glm::vec3(0, 0, -20), glm::vec3(0, 0, 0), 1024,
//                                         1024, 80.2f, render_target_options.ray_depth);
//             settings = cr::display::user_input();
//             triangular_scenes = std::vector<std::unique_ptr<cr::triangular_scene>>();
//             scenes = std::vector<cr::scene<cr::triangular_scene>>();
//             frame = cr::atomic_image(configuration.width(), configuration.height());
//             normal_frame = cr::atomic_image(configuration.width(), configuration.height());
//             albedo_frame = cr::atomic_image(configuration.width(), configuration.height());
//             sample_count = uint64_t(0);
//         }
//     };
// }