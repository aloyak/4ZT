#include <4ZT/engine.h>

// Simple demo application that uses the 4ZT raytracer engine
// No GUI or windowing, renders whats in the scene
int main() {

    FourZT::Settings settings;
    settings.backend = FourZT::Backend::CPU;
    settings.outputHeight = 500;
    settings.aspectRatio = 1;

    settings.samplesPerPixel = 100;
    settings.maxDepth = 50;

    settings.targetThreads = 16;

    FourZT::Engine engine(settings);

    FourZT::Camera camera(
        Point3(278, 278, -800),
        Point3(278, 278, 0),
        40.0
    );
    camera.useSkybox = false;

    FourZT::World world;

    // empty cornel box
    auto red = std::make_shared<FourZT::Lambertian>(Color(0.65, 0.05, 0.05));
    auto green = std::make_shared<FourZT::Lambertian>(Color(0.12, 0.45, 0.15));
    auto white = std::make_shared<FourZT::Lambertian>(Color(0.73, 0.73, 0.73));

    //world.add(make_shared<quad>(point3(555,0,0), vec3(0,555,0), vec3(0,0,555), green));
    //world.add(make_shared<quad>(point3(0,0,0), vec3(0,555,0), vec3(0,0,555), red));
    //world.add(make_shared<quad>(point3(343, 554, 332), vec3(-130,0,0), vec3(0,0,-105), light));
    //world.add(make_shared<quad>(point3(0,0,0), vec3(555,0,0), vec3(0,0,555), white));
    //world.add(make_shared<quad>(point3(555,555,555), vec3(-555,0,0), vec3(0,0,-555), white));
    //world.add(make_shared<quad>(point3(0,0,555), vec3(555,0,0), vec3(0,555,0), white));

    world.Add<FourZT::Quad>(Point3(555, 0, 0), Vec3(0, 555, 0), Vec3(0, 0, 555), green);
    world.Add<FourZT::Quad>(Point3(0, 0, 0), Vec3(0, 555, 0), Vec3(0, 0, 555), red);
    world.Add<FourZT::Quad>(Point3(0, 0, 0), Vec3(555, 0, 0), Vec3(0, 0, 555), white);
    world.Add<FourZT::Quad>(Point3(555, 555, 555), Vec3(-555, 0, 0), Vec3(0, 0, -555), white);
    world.Add<FourZT::Quad>(Point3(0, 0, 555), Vec3(555, 0, 0), Vec3(0, 555, 0), white);

    world.Add<FourZT::Box>(Point3(347.5, 165, 377.5), Vec3(165, 330, 165), Vec3(0, torads(15), 0), white);
    world.Add<FourZT::Box>(Point3(212.5, 82.5, 147.5), Vec3(165, 165, 165), Vec3(0, torads(-18), 0), white);
    
    auto light = std::make_shared<FourZT::Emissive>(Color(15, 15, 15));
    world.Add<FourZT::Quad>(Point3(343, 554, 332), Vec3(-130, 0, 0), Vec3(0, 0, -105), light);
    //world.Add<FourZT::Sphere>(Point3(343, 554, 332), 105, light);
    FourZT::Image image = engine.Render(camera, world);
    //image.Save("out.ppm");
    image.PrintOut();
}
