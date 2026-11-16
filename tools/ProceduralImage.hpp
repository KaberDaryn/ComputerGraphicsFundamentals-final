#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <vector>

namespace procedural {
using Color=std::array<std::uint8_t,3>;
struct Point { int x,y; };

// Small RGB raster for the optional C++ asset tools. No window or GPU needed.
class Image {
public:
    int width,height;
    std::vector<std::uint8_t> pixels;
    Image(int w,int h,Color background={0,0,0}):width(w),height(h) {
        if(w<=0 || h<=0 || w>8192 || h>8192) throw std::invalid_argument("Invalid image size");
        pixels.resize(static_cast<std::size_t>(w)*h*3);
        rect(0,0,w-1,h-1,background);
    }
    Color get(int x,int y) const {
        const auto p=(static_cast<std::size_t>(y)*width+x)*3;
        return {pixels[p],pixels[p+1],pixels[p+2]};
    }
    void set(int x,int y,Color c) {
        if(x<0 || y<0 || x>=width || y>=height) return;
        const auto p=(static_cast<std::size_t>(y)*width+x)*3;
        std::copy(c.begin(),c.end(),pixels.begin()+p);
    }
    void rect(int x0,int y0,int x1,int y1,Color c) {
        for(int y=std::max(0,y0);y<=std::min(height-1,y1);++y)
            for(int x=std::max(0,x0);x<=std::min(width-1,x1);++x) set(x,y,c);
    }
    void ellipse(int x0,int y0,int x1,int y1,Color c) {
        const double rx=(x1-x0)*.5,ry=(y1-y0)*.5,cx=x0+rx,cy=y0+ry;
        if(rx<=0 || ry<=0) return;
        for(int y=std::max(0,y0);y<=std::min(height-1,y1);++y)
            for(int x=std::max(0,x0);x<=std::min(width-1,x1);++x)
                if(std::pow((x-cx)/rx,2)+std::pow((y-cy)/ry,2)<=1) set(x,y,c);
    }
    void polygon(std::initializer_list<Point> points,Color c) {
        const std::vector<Point> v(points);
        if(v.size()<3) return;
        int low=height,high=0;
        for(auto p:v) { low=std::min(low,p.y); high=std::max(high,p.y); }
        for(int y=std::max(0,low);y<=std::min(height-1,high);++y) {
            std::vector<double> intersections;
            for(std::size_t i=0,j=v.size()-1;i<v.size();j=i++) {
                const auto a=v[i],b=v[j];
                if((a.y<=y && b.y>y)||(b.y<=y && a.y>y))
                    intersections.push_back(a.x+double(y-a.y)*(b.x-a.x)/(b.y-a.y));
            }
            std::sort(intersections.begin(),intersections.end());
            for(std::size_t i=0;i+1<intersections.size();i+=2)
                rect(int(std::ceil(intersections[i])),y,int(std::floor(intersections[i+1])),y,c);
        }
    }
    void line(Point a,Point b,int thickness,Color c) {
        const int count=std::max(std::abs(b.x-a.x),std::abs(b.y-a.y));
        for(int i=0;i<=count;++i) {
            const float t=count?float(i)/count:0.f;
            const int x=int(std::lround(a.x+(b.x-a.x)*t)),y=int(std::lround(a.y+(b.y-a.y)*t));
            rect(x-thickness/2,y-thickness/2,x+thickness/2,y+thickness/2,c);
        }
    }
    void diamond(int x,int y,int rx,int ry,Color c) {
        polygon({{x,y-ry},{x+rx,y},{x,y+ry},{x-rx,y}},c);
    }
    void text(int x,int y,const std::string& value,int scale,Color c) {
        // Original 5x7 uppercase bitmap alphabet: independent of installed fonts.
        static const std::string alphabet="ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        static constexpr std::uint8_t glyphs[][7]={
            {14,17,17,31,17,17,17},{30,17,17,30,17,17,30},{14,17,16,16,16,17,14},
            {30,17,17,17,17,17,30},{31,16,16,30,16,16,31},{31,16,16,30,16,16,16},
            {14,17,16,23,17,17,15},{17,17,17,31,17,17,17},{14,4,4,4,4,4,14},
            {7,2,2,2,18,18,12},{17,18,20,24,20,18,17},{16,16,16,16,16,16,31},
            {17,27,21,21,17,17,17},{17,25,21,19,17,17,17},{14,17,17,17,17,17,14},
            {30,17,17,30,16,16,16},{14,17,17,17,21,18,13},{30,17,17,30,20,18,17},
            {15,16,16,14,1,1,30},{31,4,4,4,4,4,4},{17,17,17,17,17,17,14},
            {17,17,17,17,17,10,4},{17,17,17,21,21,21,10},{17,17,10,4,10,17,17},
            {17,17,10,4,4,4,4},{31,1,2,4,8,16,31},{14,17,19,21,25,17,14},
            {4,12,4,4,4,4,14},{14,17,1,2,4,8,31},{30,1,1,14,1,1,30},
            {2,6,10,18,31,2,2},{31,16,16,30,1,1,30},{14,16,16,30,17,17,14},
            {31,1,2,4,8,8,8},{14,17,17,14,17,17,14},{14,17,17,15,1,1,14}};
        for(unsigned char original:value) {
            const char ch=original>='a' && original<='z'?char(original-'a'+'A'):char(original);
            const auto index=alphabet.find(ch);
            if(index!=std::string::npos) {
                for(int row=0;row<7;++row) for(int col=0;col<5;++col)
                    if(glyphs[index][row]&(1<<(4-col)))
                        rect(x+col*scale,y+row*scale,x+(col+1)*scale-1,y+(row+1)*scale-1,c);
            } else if(ch!=' ') {
                if(ch=='.' || ch==':' || ch==';') rect(x+2*scale,y+6*scale,x+3*scale-1,y+7*scale-1,c);
                if(ch==':') rect(x+2*scale,y+2*scale,x+3*scale-1,y+3*scale-1,c);
                if(ch=='-' || ch=='=' || ch=='+') rect(x,y+3*scale,x+5*scale-1,y+4*scale-1,c);
                if(ch=='/' || ch=='$') line({x,y+6*scale},{x+4*scale,y},scale,c);
                if(ch=='_' ) rect(x,y+6*scale,x+5*scale-1,y+7*scale-1,c);
            }
            x+=6*scale;
        }
    }
    void save(const std::filesystem::path& path) const {
        // PNG RGB8, filter 0; zlib uses stored DEFLATE blocks (no compressor dependency).
        // Formats: https://www.w3.org/TR/png-3/ and RFC 1950 / RFC 1951.
        std::vector<std::uint8_t> raw;
        raw.reserve((static_cast<std::size_t>(width)*3+1)*height);
        for(int y=0;y<height;++y) {
            raw.push_back(0);
            const auto begin=pixels.begin()+static_cast<std::size_t>(y)*width*3;
            raw.insert(raw.end(),begin,begin+width*3);
        }
        std::vector<std::uint8_t> compressed{0x78,0x01};
        for(std::size_t at=0;at<raw.size();) {
            const auto count=static_cast<std::uint16_t>(std::min<std::size_t>(65535,raw.size()-at));
            const auto inverse=static_cast<std::uint16_t>(~count);
            compressed.push_back(at+count==raw.size()?1:0);
            for(auto n:{count,inverse}) { compressed.push_back(n&255); compressed.push_back(n>>8); }
            compressed.insert(compressed.end(),raw.begin()+at,raw.begin()+at+count);
            at+=count;
        }
        std::uint32_t a=1,b=0;
        for(auto byte:raw) { a=(a+byte)%65521; b=(b+a)%65521; }
        append32(compressed,(b<<16)|a);
        std::vector<std::uint8_t> png{137,80,78,71,13,10,26,10},header;
        append32(header,width); append32(header,height);
        header.insert(header.end(),{8,2,0,0,0});
        chunk(png,"IHDR",header); chunk(png,"IDAT",compressed); chunk(png,"IEND",{});
        if(!path.parent_path().empty()) std::filesystem::create_directories(path.parent_path());
        std::ofstream output(path,std::ios::binary);
        output.write(reinterpret_cast<const char*>(png.data()),static_cast<std::streamsize>(png.size()));
        if(!output) throw std::runtime_error("Cannot write PNG: "+path.string());
    }
private:
    static void append32(std::vector<std::uint8_t>& data,std::uint32_t value) {
        for(int shift:{24,16,8,0}) data.push_back((value>>shift)&255);
    }
    static void chunk(std::vector<std::uint8_t>& png,const char* type,const std::vector<std::uint8_t>& data) {
        append32(png,static_cast<std::uint32_t>(data.size()));
        const auto start=png.size();
        png.insert(png.end(),type,type+4); png.insert(png.end(),data.begin(),data.end());
        std::uint32_t crc=0xffffffffu;
        for(std::size_t i=start;i<png.size();++i) {
            crc^=png[i];
            for(int bit=0;bit<8;++bit) crc=(crc>>1)^((crc&1)?0xedb88320u:0u);
        }
        append32(png,crc^0xffffffffu);
    }
};
inline Color shade(Color color,int amount) {
    for(auto& channel:color) channel=static_cast<std::uint8_t>(std::clamp(int(channel)+amount,0,255));
    return color;
}
}
