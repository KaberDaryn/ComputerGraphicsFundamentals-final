#include "scene/SceneFactory.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

namespace SceneFactory {
namespace {
const glm::vec3 oak(.62f,.43f,.26f), cream(.83f,.76f,.63f), ink(.055f,.075f,.09f);
SceneNode& yawGroup(SceneNode& p,const char* n,glm::vec3 at,float yaw) {
    auto& g=group(p,n,at);
    g.localTransform=glm::translate(glm::mat4(1),at)*
        glm::rotate(glm::mat4(1),glm::radians(yaw),glm::vec3(0,1,0));
    return g;
}
SceneNode& rod(SceneNode& p,const char* name,Mesh* mesh,glm::vec3 from,glm::vec3 to,
               float width,glm::vec3 color) {
    auto& n=part(p,name,mesh,(from+to)*.5f,{width,glm::length(to-from),width},color,nullptr,{1,1},20);
    const glm::vec3 direction=glm::normalize(to-from);
    const glm::vec3 axis=glm::cross(glm::vec3(0,1,0),direction);
    glm::mat4 rotation(1);
    if(glm::length(axis)>.0001f)
        rotation=glm::rotate(rotation,std::acos(glm::clamp(direction.y,-1.f,1.f)),glm::normalize(axis));
    else if(direction.y<0)
        rotation=glm::rotate(rotation,3.14159265f,glm::vec3(1,0,0));
    n.localTransform=glm::translate(glm::mat4(1),(from+to)*.5f)*rotation*
        glm::scale(glm::mat4(1),glm::vec3(width,glm::length(to-from),width));
    return n;
}
void book(SceneNode& p,const SceneAssets& a,glm::vec3 at,glm::vec3 size,glm::vec3 color) {
    part(p,"Book cover",a.cube,at,size,color,nullptr,{1,1},5);
    part(p,"Book pages",a.cube,at+glm::vec3(.006f,0,.009f),size*glm::vec3(.86f,.78f,1),cream,nullptr,{1,1},2);
}
}

void buildStudio(SceneNode& root,const SceneAssets& a) {
    // A left-wall lounge leaves the main desk approach and centre rug clear.
    auto& sofa=yawGroup(root,"Lounge sofa",{-3.88f,0,1.60f},90);
    for(float x:{-1.05f,1.05f}) for(float z:{-.33f,.33f})
        part(sofa,"Sofa oak leg",a.cylinder,{x,.13f,z},{.10f,.27f,.10f},oak,a.wood,{1,1},12);
    part(sofa,"Sofa lower frame",a.cube,{0,.33f,0},{2.55f,.25f,1.04f},{.22f,.32f,.30f},a.fabric,{4,2},4);
    part(sofa,"Upholstered back",a.cube,{0,.86f,-.43f},{2.42f,.78f,.24f},{.26f,.39f,.36f},a.fabric,{4,2},4);
    for(float x:{-1.23f,1.23f})
        part(sofa,"Rounded sofa arm",a.sphere,{x,.65f,0},{.29f,.70f,1.10f},{.27f,.40f,.37f},a.fabric,{2,3},4);
    for(float x:{-.60f,.60f})
        part(sofa,"Deep seat cushion",a.cube,{x,.55f,.055f},{1.14f,.22f,.81f},{.33f,.46f,.42f},a.fabric,{3,2},3);
    part(sofa,"Ochre cushion",a.sphere,{-.62f,.91f,-.18f},{.70f,.61f,.27f},{.78f,.52f,.22f},a.fabric,{2,2},3);
    part(sofa,"Ornament cushion",a.sphere,{.48f,.90f,-.18f},{.69f,.60f,.27f},{1,1,1},a.rug,{1,1},3);
    part(sofa,"Woven throw across seat",a.cube,{.80f,.667f,.055f},{.50f,.018f,.77f},{1,1,1},a.rug,{1,1},2);
    part(sofa,"Hanging throw",a.cube,{.80f,.46f,.452f},{.50f,.40f,.018f},{.68f,.40f,.25f},a.fabric,{2,2},2);

    auto& table=group(root,"Lounge coffee table",{-2.30f,0,1.65f});
    for(float x:{-.28f,.28f}) for(float z:{-.45f,.45f})
        part(table,"Coffee table leg",a.cylinder,{x,.23f,z},{.055f,.45f,.055f},ink,nullptr,{1,1},55);
    part(table,"Oak coffee tabletop",a.cube,{0,.49f,0},{.82f,.075f,1.27f},oak,a.wood,{1,1},18);
    book(table,a,{-.08f,.55f,-.18f},{.47f,.055f,.55f},{.26f,.45f,.49f});
    part(table,"Tea saucer",a.cylinder,{.10f,.545f,.33f},{.25f,.025f,.25f},cream,nullptr,{1,1},26);
    part(table,"Tea bowl",a.sphere,{.10f,.60f,.33f},{.19f,.11f,.19f},{.63f,.32f,.20f},nullptr,{1,1},28);

    auto& shelves=group(root,"Tall bookcase",{-3.80f,0,-2.72f});
    for(float x:{-.66f,.66f})
        part(shelves,"Bookcase oak side",a.cube,{x,1.1f,0},{.08f,2.20f,.52f},oak,a.wood,{1,3},12);
    part(shelves,"Bookcase backing",a.cube,{0,1.1f,-.23f},{1.32f,2.20f,.045f},{.26f,.33f,.30f},nullptr,{1,1},5);
    const glm::vec3 colors[]={{.53f,.25f,.17f},{.24f,.41f,.40f},{.73f,.59f,.36f},{.32f,.36f,.46f}};
    for(int row=0;row<5;++row) {
        const float y=.08f+row*.52f;
        part(shelves,"Bookcase shelf",a.cube,{0,y,0},{1.32f,.055f,.53f},oak,a.wood,{2,1},12);
        if(row<3) for(int b=0;b<5;++b)
            book(shelves,a,{-.49f+b*.17f,y+.19f,0},{.13f,.32f+.025f*(b%2),.31f},colors[(row+b)%4]);
        if(row==3) {
            part(shelves,"Linen storage box",a.cube,{-.26f,y+.19f,0},{.56f,.32f,.40f},{.65f,.61f,.49f},a.fabric,{2,1},3);
            part(shelves,"Box label",a.cube,{-.26f,y+.20f,.206f},{.17f,.065f,.008f},cream,nullptr,{1,1},2);
            part(shelves,"Decorative stone",a.sphere,{.33f,y+.12f,0},{.29f,.19f,.24f},{.58f,.47f,.35f},nullptr,{1,1},12);
        }
    }

    // Framed original mountain artwork on the side wall.
    auto& art=yawGroup(root,"Alatau mountain print",{-4.59f,2.98f,-1.56f},90);
    part(art,"Mountain picture frame",a.cube,{0,0,0},{1.55f,1.02f,.055f},oak,a.wood,{1,1},18);
    part(art,"Picture mat",a.cube,{0,0,.032f},{1.43f,.90f,.012f},cream,nullptr,{1,1},2);
    part(art,"Alatau artwork",a.screen,{0,0,.041f},{1.31f,.79f,1},{1,1,1},a.mountains,{1,1},3);

    auto& lamp=group(root,"Reading floor lamp",{-3.80f,0,3.45f});
    part(lamp,"Floor lamp base",a.cylinder,{0,.04f,0},{.48f,.08f,.48f},ink,nullptr,{1,1},65);
    part(lamp,"Brass floor lamp stem",a.cylinder,{0,.92f,0},{.034f,1.80f,.034f},{.54f,.39f,.19f},nullptr,{1,1},85);
    part(lamp,"Linen lamp shade",a.cylinder,{0,1.83f,0},{.62f,.46f,.62f},cream,a.fabric,{3,1},4);
    auto& bulb=part(lamp,"Reading lamp glow",a.sphere,{0,1.60f,0},{.40f,.07f,.40f},{1,.75f,.40f});
    bulb.castsShadow=false;

    // A compact cabinet occupies the right underside of the desk.
    auto& cabinet=group(root,"Office cabinet",{1.12f,0,-.18f});
    for(float x:{-.34f,.34f})
        part(cabinet,"Cabinet side",a.cube,{x,.43f,0},{.045f,.79f,.70f},oak,a.wood,{1,1},14);
    part(cabinet,"Cabinet top",a.cube,{0,.84f,0},{.72f,.045f,.70f},oak,a.wood,{1,1},14);
    part(cabinet,"Cabinet back",a.cube,{0,.43f,-.33f},{.65f,.79f,.035f},oak,a.wood,{1,1},14);
    for(int i=0;i<3;++i) {
        auto& drawer=group(cabinet,("Office drawer "+std::to_string(i)).c_str(),{0,.17f+i*.25f,.17f});
        part(drawer,"Drawer bottom",a.cube,{0,-.07f,-.13f},{.61f,.025f,.56f},{.52f,.40f,.27f},a.wood,{1,1},8);
        for(float x:{-.30f,.30f})
            part(drawer,"Drawer side",a.cube,{x,0,-.13f},{.025f,.16f,.56f},oak,a.wood,{1,1},12);
        part(drawer,"Drawer front",a.cube,{0,0,.16f},{.65f,.23f,.04f},{.37f,.48f,.44f},nullptr,{1,1},12);
        part(drawer,"Drawer brass pull",a.cube,{0,.02f,.20f},{.15f,.025f,.035f},{.66f,.50f,.27f},nullptr,{1,1},65);
    }

    auto& pc=group(root,"Developer PC",{2.40f,0,-.05f});
    part(pc,"PC case",a.cube,{0,.51f,0},{.52f,.99f,.86f},ink,nullptr,{1,1},60);
    // An open side recess holds components behind one genuine transparent pane.
    part(pc,"PC side recess",a.cube,{.267f,.53f,0},{.012f,.85f,.72f},{.025f,.035f,.045f});
    part(pc,"Graphics card",a.cube,{.285f,.39f,-.02f},{.045f,.14f,.52f},{.22f,.25f,.26f});
    for(float z:{-.19f,.19f}) {
        auto& fan=part(pc,"Internal RGB ring",a.torus,{.31f,.68f,z},{.29f,.29f,.29f},{1,1,1});
        fan.localTransform=glm::translate(glm::mat4(1),glm::vec3(.31f,.68f,z))*
            glm::rotate(glm::mat4(1),glm::radians(90.f),glm::vec3(0,1,0))*
            glm::scale(glm::mat4(1),glm::vec3(.29f));
        fan.renderPass=RenderPass::RGB; fan.rgbPhase=z;
    }
    for(int i=0;i<3;++i) {
        part(pc,"PC fan recess",a.sphere,{0,.25f+i*.27f,.438f},{.31f,.31f,.015f},{.02f,.03f,.04f});
        auto& ring=part(pc,"Front RGB fan ring",a.torus,{0,.25f+i*.27f,.45f},{.32f,.32f,.32f},{1,1,1});
        ring.renderPass=RenderPass::RGB; ring.rgbPhase=i*.20f;
    }
    auto& glass=part(pc,"Smoked glass side",a.screen,{.335f,.53f,0},{.72f,.85f,1},{.24f,.42f,.48f},nullptr,{1,1},110);
    glass.localTransform=glm::translate(glm::mat4(1),glm::vec3(.335f,.53f,0))*
        glm::rotate(glm::mat4(1),glm::radians(90.f),glm::vec3(0,1,0))*
        glm::scale(glm::mat4(1),glm::vec3(.72f,.85f,1));
    glass.renderPass=RenderPass::Glass; glass.opacity=.22f; glass.castsShadow=false;

    part(pc,"Headphone stand foot",a.cube,{0,1.02f,0},{.26f,.025f,.24f},oak,a.wood);
    part(pc,"Headphone stand",a.cylinder,{0,1.20f,0},{.025f,.36f,.025f},ink);
    part(pc,"Headphone band",a.torus,{0,1.43f,0},{.39f,.44f,.30f},ink,nullptr,{1,1},45);
    for(float x:{-.14f,.14f})
        part(pc,"Headphone earcup",a.sphere,{x,1.31f,.02f},{.10f,.18f,.12f},{.13f,.18f,.19f},nullptr,{1,1},20);

    for(float x:{-1.70f,1.70f}) {
        auto& speaker=group(root,"Desktop speaker",{x,1.12f,-.69f});
        part(speaker,"Speaker cabinet",a.cube,{0,.16f,0},{.19f,.32f,.22f},ink,nullptr,{1,1},30);
        part(speaker,"Speaker woofer",a.torus,{0,.12f,.118f},{.16f,.16f,.12f},{.40f,.43f,.43f});
        part(speaker,"Speaker tweeter",a.sphere,{0,.245f,.119f},{.044f,.044f,.012f},{.56f,.56f,.51f});
    }

    // Clock face sits above the original wall shelves.
    auto& clock=group(root,"Wall clock",{-2.58f,3.85f,-3.30f});
    auto& face=part(clock,"Clock face",a.cylinder,{0,0,0},{.52f,.035f,.52f},cream,nullptr,{1,1},12);
    face.localTransform=glm::rotate(glm::mat4(1),glm::radians(90.f),glm::vec3(1,0,0))*
        glm::scale(glm::mat4(1),glm::vec3(.52f,.035f,.52f));
    part(clock,"Clock oak rim",a.torus,{0,0,.018f},{.63f,.63f,.24f},oak,a.wood);
    rod(clock,"Clock minute hand",a.cylinder,{0,0,.026f},{.05f,.18f,.026f},.012f,ink);
    rod(clock,"Clock hour hand",a.cylinder,{0,0,.03f},{-.12f,.035f,.03f},.017f,ink);
    for(int i=0;i<12;++i) {
        const float t=i*6.2831853f/12;
        part(clock,"Clock hour marker",a.sphere,{.21f*std::sin(t),.21f*std::cos(t),.026f},{.018f,.018f,.008f},ink);
    }

    auto& board=group(root,"Idea corkboard",{4.35f,2.7f,-3.32f});
    part(board,"Corkboard frame",a.cube,{0,0,0},{.88f,1.22f,.05f},oak,a.wood);
    part(board,"Cork surface",a.cube,{0,0,.03f},{.80f,1.14f,.025f},{.56f,.40f,.24f},a.fabric,{3,3},3);
    for(int i=0;i<4;++i) {
        const float x=(i%2-.5f)*.35f,y=(i/2-.5f)*.44f;
        part(board,"Pinned note",a.cube,{x,y,.047f},{.27f,.32f,.008f},i%2?cream:glm::vec3(.69f,.73f,.54f));
        part(board,"Note pin",a.sphere,{x,y+.13f,.058f},{.028f,.028f,.018f},{.65f,.25f,.18f});
    }

    // Small details that anchor the room to daily use.
    auto& bag=group(root,"Everyday backpack",{1.65f,0,2.8f});
    part(bag,"Backpack body",a.sphere,{0,.28f,0},{.46f,.55f,.25f},{.25f,.33f,.31f},a.fabric,{2,2},4);
    part(bag,"Backpack pocket",a.sphere,{0,.22f,.13f},{.34f,.27f,.10f},{.32f,.41f,.37f},a.fabric,{2,2},4);
    part(bag,"Backpack handle",a.torus,{0,.54f,0},{.17f,.15f,.12f},ink);
    part(root,"Wall outlet",a.cube,{1.3f,.28f,-3.36f},{.23f,.13f,.025f},cream);
    for(float x:{1.24f,1.36f})
        part(root,"Outlet socket",a.sphere,{x,.28f,-3.342f},{.045f,.045f,.006f},ink);
    rod(root,"PC floor cable",a.cylinder,{2.4f,.025f,-.50f},{2.10f,.025f,-2.7f},.013f,ink);
    rod(root,"Cable to outlet",a.cylinder,{2.10f,.025f,-2.7f},{1.30f,.28f,-3.32f},.013f,ink);

    auto& strip=part(root,"Shelf warm light",a.cube,{-2.55f,2.87f,-2.94f},{1.57f,.016f,.045f},{1,.70f,.32f});
    strip.castsShadow=false;

    auto& sillPlant=group(root,"Window sill plant",{2.96f,1.96f,-3.02f});
    part(sillPlant,"Window ceramic pot",a.cylinder,{0,.12f,0},{.23f,.24f,.23f},{.76f,.68f,.51f},nullptr,{1,1},18);
    part(sillPlant,"Window plant stem",a.cylinder,{0,.31f,0},{.018f,.25f,.018f},{.18f,.32f,.17f});
    for(int i=0;i<4;++i) {
        const float t=i*1.57f;
        part(sillPlant,"Window plant leaf",a.sphere,{.075f*std::cos(t),.35f+i*.025f,.075f*std::sin(t)},
             {.17f,.10f,.12f},{.23f,.42f,.23f},nullptr,{1,1},8);
    }
    auto& photo=group(shelves,"Bookcase postcard",{-.22f,2.37f,.10f});
    part(photo,"Postcard oak frame",a.cube,{0,0,0},{.38f,.32f,.04f},oak,a.wood);
    part(photo,"Travel postcard",a.screen,{0,0,.026f},{.32f,.26f,1},{1,1,1},a.mountains,{1,1},3);

    auto& digital=group(root,"Desktop digital clock",{0,1.21f,-.70f});
    part(digital,"Digital clock body",a.cube,{0,0,0},{.39f,.15f,.12f},ink,nullptr,{1,1},35);
    const int digits[]={0x06,0x3f,0x5b,0x66}; // 10:24, a decorative display.
    const glm::vec2 segments[]={{0,.042f},{.022f,.021f},{.022f,-.021f},{0,-.042f},
                                {-.022f,-.021f},{-.022f,.021f},{0,0}};
    for(int digit=0;digit<4;++digit) for(int s=0;s<7;++s) if(digits[digit]&(1<<s)) {
        auto& bar=part(digital,"Clock LED segment",a.cube,
            {-.13f+digit*.085f+segments[s].x,segments[s].y,.063f},
            s==0||s==3||s==6?glm::vec3(.036f,.006f,.003f):glm::vec3(.006f,.030f,.003f),
            {.31f,.62f,.52f});
        bar.emission={.15f,.38f,.28f}; bar.castsShadow=false;
    }
}
}
