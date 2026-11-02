#include "IconButton.h"
#include "icons.h"
#include <algorithm>
#include <cassert>
#include <climits>
#include <cstdio>
#include <cstring>
#include <random>
#include <vector>

struct RunCanvas : Canvas {
    RunCanvas() : Canvas(TFT_PORTRAIT, BLACK) {}
    void init() override {}
    Point* getTouchedPoint() override { return nullptr; }
};
struct Counts { unsigned pixels=0, runs=0, reads=0; };
static unsigned cases=0;

// Independent widened per-pixel oracle: no run renderer or production clipping.
static Counts reference(Guitft& panel, int16_t x, int16_t y,
                        const uint16_t* bitmap, int16_t w, int16_t h,
                        bool transparent, uint16_t key) {
    Counts count;
    if (!bitmap || w<=0 || h<=0) return count;
    panel.startWrite();
    for(int32_t j=0;j<h;++j) {
        bool previousDrawn=false;
        uint16_t previous=0;
        for(int32_t i=0;i<w;++i) {
            const int32_t px=int32_t(x)+i, py=int32_t(y)+j;
            if(px<0 || py<0 || px>=panel.width() || py>=panel.height()) {
                previousDrawn=false; continue;
            }
            ++count.reads;
            const auto color=pgm_read_word(bitmap+j*int32_t(w)+i);
            if(transparent && color==key) { previousDrawn=false; continue; }
            if(!previousDrawn || previous!=color) ++count.runs;
            panel.writePixel(int16_t(px),int16_t(py),color);
            ++count.pixels; previousDrawn=true; previous=color;
        }
    }
    panel.endWrite();
    return count;
}
static Counts check(RunCanvas& canvas, int rotation, const uint16_t* bitmap,
                    int16_t w,int16_t h,int16_t x,int16_t y,bool transparent,
                    uint16_t key=BLACK,bool baseline=false,const char* name=nullptr) {
    auto& panel=*canvas.tft;
    panel.setRotation(rotation);
    std::fill(panel.pixels.begin(),panel.pixels.end(),uint16_t(0x5a69));
    panel.resetCounts();
    Guitft oracle(-1,-1); oracle.setRotation(rotation);
    std::fill(oracle.pixels.begin(),oracle.pixels.end(),uint16_t(0x5a69));
    const auto expected=reference(oracle,x,y,bitmap,w,h,transparent,key);
    IconButton icon(w,h,bitmap); icon.setCanvas(&canvas); icon.useTransparency=transparent;
    iconReadBase=bitmap; iconReadCapacity=w>0 && h>0 ? size_t(w)*h : 0; iconReads=0;
    icon.drawRGBABitmap(x,y,bitmap,w,h,key);
    iconReadBase=nullptr;
    assert(panel.pixels==oracle.pixels && "run raster must match per-pixel RGB565 oracle");
    assert(panel.depth==0 && panel.starts==panel.ends);
    if(!baseline) {
        assert(iconReads==expected.reads && "one flash read per visible source pixel only");
        assert(panel.pixelWrites==0 && panel.lineWrites==expected.runs);
        assert(panel.drawLineWrappers==0 && "use writeFastHLine directly when the backend provides it");
        assert(panel.starts==(expected.reads ? 1u : 0u));
    } else {
        assert(panel.pixelWrites==expected.pixels && panel.lineWrites==0);
    }
    if(name) std::printf("%s %s transparency=%d pixels=%u runs=%u actual_pixel_writes=%u actual_hlines=%u flash_reads=%zu transactions=%u\n",
        baseline?"BASELINE":"OPTIMIZED",name,transparent,expected.pixels,expected.runs,
        panel.pixelWrites,panel.lineWrites,iconReads,panel.starts);
    ++cases;
    return expected;
}
static void assets(RunCanvas& canvas,bool baseline) {
    struct Asset { const char* name; const uint16_t* data; int16_t w,h; };
    const Asset images[]={
        {"slider_on",slider_on,60,30},{"slider_off",slider_off,60,30},
        {"on_full",on_full,50,50},{"off_full",off_full,50,50},
        {"lightbulb_on",lightbulb_on,50,50},{"lightbulb_off",lightbulb_off,50,50}
    };
    for(const auto& image:images) for(bool transparent:{false,true}) {
        for(int rotation=0;rotation<4;++rotation) {
            check(canvas,rotation,image.data,image.w,image.h,10,20,transparent,BLACK,baseline,
                  rotation==0?image.name:nullptr);
            if(!baseline) for(const auto& position:std::vector<std::pair<int,int>>{
                {-17,-9},{235,315},{315,235},{-image.w+1,-image.h+1}})
                check(canvas,rotation,image.data,image.w,image.h,position.first,position.second,transparent);
        }
    }
}
static void adversarial(RunCanvas& canvas) {
    std::mt19937 random(0x1c0b17u);
    for(int width:{1,2,3,7,31,60,67}) for(int height:{1,2,9}) {
        std::vector<uint16_t> image(size_t(width)*height);
        for(int pattern=0;pattern<6;++pattern) {
            for(size_t i=0;i<image.size();++i) {
                switch(pattern) {
                    case 0: image[i]=RED; break; // long solid runs, remainder widths
                    case 1: image[i]=BLACK; break; // fully transparent
                    case 2: image[i]=i%2?BLUE:RED; break; // no same-color neighbors
                    case 3: image[i]=i%3?GREEN:BLACK; break; // transparent holes
                    case 4: image[i]=uint16_t(random()); break; // arbitrary RGB565
                    case 5: image[i]=uint16_t((random()%5)*0x1111); break;
                }
            }
            for(int rotation=0;rotation<4;++rotation) for(bool transparent:{false,true})
            for(const auto& position:std::vector<std::pair<int,int>>{
                {0,0},{-1,-1},{239,319},{319,239},{240,320},
                {-width,0},{0,-height},{INT16_MIN,INT16_MIN},
                {INT16_MAX,INT16_MAX},{INT16_MAX,0},{0,INT16_MAX}})
                check(canvas,rotation,image.data(),width,height,position.first,position.second,
                      transparent,pattern==5?0x2222:BLACK);
        }
    }
    // Visible row offsets exceed signed 16-bit, and source columns/rows extend
    // to INT16_MAX. Capacity remains the caller's contract; ASan guards it.
    std::vector<uint16_t> wide(32767*2), tall(2*32767), square(257*257);
    for(auto* image:{&wide,&tall,&square})
        for(size_t i=0;i<image->size();++i) (*image)[i]=uint16_t(i*37u);
    for(int rotation=0;rotation<4;++rotation) for(bool transparent:{false,true}) {
        check(canvas,rotation,wide.data(),32767,2,-32760,0,transparent);
        check(canvas,rotation,tall.data(),2,32767,0,-32760,transparent);
        check(canvas,rotation,square.data(),257,257,-200,-200,transparent);
    }
}
static void noops(RunCanvas& canvas) {
    const uint16_t pixel=RED;
    for(const auto& dimensions:std::vector<std::pair<int,int>>{{0,1},{1,0},{-1,1},{1,-1},{INT16_MIN,1}})
        check(canvas,0,&pixel,dimensions.first,dimensions.second,0,0,true);
    check(canvas,0,nullptr,1,1,0,0,true);
    auto& panel=*canvas.tft;
    IconButton hidden(1,1,&pixel); hidden.setCanvas(&canvas); hidden.visible=false;
    panel.resetCounts(); iconReadBase=&pixel; iconReadCapacity=1; iconReads=0;
    hidden.update();
    IconButton detached(1,1,&pixel); detached.update();
    iconReadBase=nullptr;
    assert(!iconReads && !panel.starts && !panel.lineWrites && !panel.pixelWrites);
}
int main(int argc,char** argv) {
    const bool baseline=argc==2 && std::strcmp(argv[1],"--baseline")==0;
    RunCanvas canvas;
    assets(canvas,baseline);
    if(!baseline) { adversarial(canvas); noops(canvas); }
    std::printf("icon_runs real-GFX: PASS cases=%u mode=%s sizeof(IconButton)=%zu sizeof(Button)=%zu\n",
                cases,baseline?"baseline":"optimized",sizeof(IconButton),sizeof(Button));
}
