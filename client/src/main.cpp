#include <4ZT/engine.h>

int main() {

    FourZT::Settings settings;
    settings.backend = FourZT::Backend::CPU;
    settings.outputHeight = 500;
    settings.aspectRatio = 1.5;

    settings.samplesPerPixel = 100;
    settings.maxDepth = 50;

    settings.targetThreads = 24;

    FourZT::Engine engine(settings);

    FourZT::Camera camera(
        Point3(-2, 2, 1),
        Point3(0, 0, -1),
        30.0
    );

    FourZT::World world;

    auto right = std::make_shared<FourZT::Dialectric>(1/1.33);
    auto center = std::make_shared<FourZT::Metal>(Color(0.8, 0.8, 0.8), 0.3);
    auto left = std::make_shared<FourZT::Metal>(Color(0.8, 0.6, 0.9), 0.0);
    auto ground = std::make_shared<FourZT::Lambertian>(Color(0.8, 0.8, 0.0));

    world.Add<FourZT::Sphere>(Point3(0, 0, -1), 0.5, center);
    world.Add<FourZT::Sphere>(Point3(1, 0, -1.25), 0.5, right);
    world.Add<FourZT::Sphere>(Point3(-1, 0, -1.25), 0.5, left);
    world.Add<FourZT::Sphere>(Point3(0, -100.5, -1), 100.0, ground);

    FourZT::Image image = engine.Render(camera, world);
    //image.Save("out.ppm");
    image.PrintOut();
}
