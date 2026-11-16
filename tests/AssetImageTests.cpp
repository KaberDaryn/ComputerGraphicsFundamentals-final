#include "ProceduralImage.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>

void roundTrip(int width,int height,const std::filesystem::path& output) {
    procedural::Image image(width,height);
    for(int y=0;y<height;++y) for(int x=0;x<width;++x)
        image.set(x,y,{static_cast<std::uint8_t>(x%256),static_cast<std::uint8_t>(y%256),
                       static_cast<std::uint8_t>((x+y)%256)});
    image.save(output);
    int w=0,h=0,channels=0;
    unsigned char* loaded=stbi_load(output.string().c_str(),&w,&h,&channels,3);
    if(!loaded) throw std::runtime_error("Generated PNG cannot be decoded");
    const bool correct=w==width && h==height && channels==3 &&
        std::equal(image.pixels.begin(),image.pixels.end(),loaded);
    stbi_image_free(loaded);
    if(!correct) throw std::runtime_error("PNG round trip changed pixels or dimensions");
}
int main(int argc,char** argv) {
    try {
        const auto out=std::filesystem::current_path()/"asset-image-test";
        roundTrip(3,2,out/"small.png");
        roundTrip(257,257,out/"multiple-deflate-blocks.png");
        bool rejected=false;
        try { procedural::Image invalid(0,10); } catch(const std::invalid_argument&) { rejected=true; }
        if(!rejected) throw std::runtime_error("Invalid dimensions were accepted");
        if(argc==2) {
            struct Expected { const char* name; int width,height; };
            const Expected files[]={{"rug.png",1536,1536},{"oak_floor.png",1024,1024},
                {"fabric.png",512,512},{"mountains.png",1200,760},{"desktop.png",1280,720},{"terminal.png",1280,720}};
            for(const auto& expected:files) {
                int w=0,h=0,c=0;
                const auto path=std::filesystem::path(argv[1])/expected.name;
                unsigned char* data=stbi_load(path.string().c_str(),&w,&h,&c,3);
                const bool correct=data && w==expected.width && h==expected.height && c==3;
                stbi_image_free(data);
                if(!correct) throw std::runtime_error("Invalid generated texture: "+path.string());
            }
        }
        std::cout<<"PASS: PNG round trips, DEFLATE block boundaries and generated textures\n";
        return 0;
    } catch(const std::exception& e) { std::cerr<<e.what()<<'\n'; return 1; }
}
