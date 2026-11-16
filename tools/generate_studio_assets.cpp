#include "ProceduralImage.hpp"
#include <iostream>
#include <random>
using namespace procedural;

int main(int argc,char** argv) {
    try {
        if(argc>2) throw std::invalid_argument("Usage: generate_studio_assets [output-directory]");
        const std::filesystem::path out=argc==2?argv[1]:"generated-textures";
        Image fabric(512,512);
        std::mt19937 random(2815);
        for(int y=0;y<512;++y) for(int x=0;x<512;++x)
            fabric.set(x,y,shade({213,211,200},(x%4<2?7:-5)+(y%4<2?5:-4)+int(random()%7)-3));
        fabric.save(out/"fabric.png");
        Image art(1200,760,{207,219,211});
        art.ellipse(860,75,1010,225,{224,174,105});
        art.polygon({{0,400},{165,210},{290,365},{500,120},{770,410},{950,260},{1200,420},{1200,760},{0,760}},{123,158,165});
        art.polygon({{165,210},{95,290},{160,268},{185,300},{207,265}},{234,231,208});
        art.polygon({{500,120},{380,260},{470,225},{510,255},{555,213},{635,285}},{237,233,211});
        art.polygon({{0,540},{230,340},{470,530},{740,330},{1040,580},{1200,450},{1200,760},{0,760}},{66,112,118});
        art.polygon({{0,650},{300,560},{510,640},{850,510},{1200,620},{1200,760},{0,760}},{38,77,79});
        art.text(62,654,"ALATAU",6,{237,223,189});
        art.text(65,712,"KAZAKHSTAN / MOUNTAIN STUDIES",2,{184,199,183});
        art.save(out/"mountains.png");
        Image editor(1280,720,{20,29,38});
        editor.rect(0,0,1279,46,{36,51,62});
        editor.text(24,12,"DREAM DESK / STUDIO.CPP",2,{202,212,210});
        editor.rect(0,47,205,677,{26,38,47});
        const std::string files[]={"EXPLORER","  SRC/","    APP/","    GRAPHICS/","    SCENE/","    STUDIO.CPP","  ASSETS/","  SHADERS/"};
        for(int i=0;i<8;++i) editor.text(18,76+i*37,files[i],2,{155,184,184});
        const std::string code[]={"// A ROOM FOR BUILDING THINGS","SCENE STUDIO;","STUDIO.ADD(DESK);","STUDIO.ADD(SOFA);",
            "STUDIO.SETMOOD(WARM EVENING);","","WHILE WINDOW.ISOPEN","  INPUT.UPDATE;","  LIGHTS.ANIMATE(TIME);",
            "  CAMERA.ORBIT(INPUT);","  STUDIO.RENDER(CAMERA);","END","","// MADE IN KAZAKHSTAN"};
        for(int i=0;i<14;++i) {
            const auto number=std::string(i<9?"0":"")+std::to_string(i+1);
            editor.text(222,85+i*36,number,2,{85,110,123});
            editor.text(274,85+i*36,code[i],3,i==0||i==13?Color{120,179,164}:Color{155,193,217});
        }
        editor.rect(0,678,1279,719,{41,102,105});
        editor.text(20,689,"C++17 / OPENGL 3.3 / BUILD OK / 0 ERRORS",2,{223,231,213});
        editor.save(out/"desktop.png");
        Image terminal(1280,720,{10,20,23});
        terminal.rect(0,0,1279,44,{30,43,47});
        terminal.text(24,11,"DEVELOPER / DREAM DESK / STUDIO",2,{187,209,202});
        const std::string lines[]={"$ CMAKE --BUILD BUILD --PARALLEL","100% BUILT TARGET DREAMDEVELOPERDESK","", "$ CTEST --TEST-DIR BUILD",
            "1/2 CAMERACONTROLS .... PASSED","2/2 WOWSEQUENCE ....... PASSED","","100% TESTS PASSED","", "$ DREAMDEVELOPERDESK",
            "OPENGL 3.3 CORE","STUDIO READY. MAKE SOMETHING GREAT.","","DEVELOPER / DREAM DESK $ _"};
        for(int i=0;i<14;++i) terminal.text(42,78+i*40,lines[i],3,lines[i].find('$')==0?Color{218,197,145}:Color{100,197,151});
        terminal.save(out/"terminal.png");
        std::cout<<"Saved fabric, mountain artwork and monitor PNGs to "<<out<<'\n';
        return 0;
    } catch(const std::exception& e) { std::cerr<<e.what()<<'\n'; return 1; }
}
