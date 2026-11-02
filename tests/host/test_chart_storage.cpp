#include "Trend.h"
#include "MultiTrend.h"
#include "allocation_hooks.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
struct StorageCanvas: Canvas {
 StorageCanvas():Canvas(TFT_PORTRAIT,BLACK){}
 void init() override {} Point* getTouchedPoint() override{return nullptr;}
};
static void circular(){
 for(int n: {1,2,16,32,255}) {
  Trend a(180,140,0,50,100,n), b(180,140,0,50,100,n,HistoryStorage::Circular);
  MultiTrend m(180,140,0,50,100,4,n,4,HistoryStorage::Circular);
  for(int i=0;i<n;++i) b.values[i]=i%200;
  unsigned changed=0; uint8_t old[255]; memcpy(old,b.values,n);
  b.addValue(254,false);
  for(int i=0;i<n;++i) changed+=old[i]!=b.values[i];
  assert(changed==1); // actual production insertion writes one physical sample
  for(int i=0;i<n;++i)a.values[i]=i%200;
  memcpy(old,a.values,n);a.addValue(254,false);changed=0;
  for(int i=0;i<n;++i)changed+=old[i]!=a.values[i];
  assert(changed==(unsigned)n);a.clear();
  MultiTrend one(180,140,0,50,100,4,n,4,HistoryStorage::Circular);
  for(int i=0;i<4*n;++i)one.values[i]=i%200;
  uint8_t snapshot[4*255];memcpy(snapshot,one.values,4*n);
  one.addValue(2,254,false);changed=0;
  for(int i=0;i<4*n;++i)changed+=snapshot[i]!=one.values[i];
  assert(changed==1);
  b.clear();
  MultiTrend ref(180,140,0,50,100,4,n,4);
  unsigned seed=12345;
  for(int k=0;k<3*n+17;++k){
   seed=seed*1664525u+1013904223u; uint8_t v=seed>>24;
   a.addValue(v,false); b.addValue(v,false);
   for(int i=0;i<n;++i) assert(a.values[i]==b.sample(i));
   assert(a.getMin()==b.getMin() && a.getMax()==b.getMax());
   const int series=k%4; m.addValue(series,v,false); ref.addValue(series,v,false);assert(m.latest(series)==v);
   for(int s=0;s<4;++s){
    for(int i=0;i<n;++i)assert(m.sample(s,i)==ref.values[s*n+i]);
    assert(m.getMin(s)==ref.getMin(s) && m.getMax(s)==ref.getMax(s));
   }
  }
  b.init();m.init(); assert(b.sample(0)==0 && m.latest(3)==0);
  assert(b.sample(-1)==0 && b.sample(n)==0 && m.sample(255,0)==0);
 }
}
static void failures(){
 StorageCanvas c;
 for(auto mode: {HistoryStorage::NewestFirst,HistoryStorage::Circular}){
  for(int failure: {0,1}){
   MultiTrend m(180,140,0,50,100,3,16,3,mode);m.setCanvas(&c);
   host_allocation_fail_after=failure;m.init();assert(host_allocation_fail_after==-1);
   m.addValue(1,99,false);m.clear();m.autoFit(true);m.show();m.drawTrend();m.eraseTrend();
   m.init();m.addValue(1,99,false);assert(m.latest(1)==99);
  }
  Trend t(180,140,0,50,100,16,mode);t.setCanvas(&c);
  host_allocation_fail_after=0;t.init();assert(!t.values);
  t.addValue(99);t.clear();t.autoFit(true);t.show();t.init();assert(t.values);
 }
 Trend zero(10,10,0,0,0,0,HistoryStorage::Circular);assert(zero.sample(0)==0);zero.addValue(42);
}
int main(){ circular();failures();
 StorageCanvas c; c.tft->ops.reserve(200000);
 host_heap_begin();
 {
 MultiTrend t(180,140,0,50,100,3,32,3); t.setCanvas(&c); t.x=10;t.y=10;
 const auto before=host_heap_stats(); t.show(); const auto after=host_heap_stats();
 const size_t bytes=after.requested-before.requested;
 std::printf("cache requested=%zu old int slots=%zu\n",bytes,(32*4+9)*sizeof(int));std::fflush(stdout);
 assert(bytes==(32*4+9)*(std::getenv("CHART_BASELINE")?sizeof(int):sizeof(uint16_t)));
 }
 assert(host_heap_stats().retained==0);host_heap_end();
 for(int extreme: {40000,-40000}){
  host_heap_begin();{
   MultiTrend t(180,140,0,50,100,3,32,3);t.setCanvas(&c);t.x=extreme;t.y=extreme;
   auto before=host_heap_stats();t.show();auto after=host_heap_stats();
   assert(after.requested-before.requested==(32*4+9)*sizeof(int));
  }host_heap_end();
 }
 for(auto mode: {HistoryStorage::NewestFirst,HistoryStorage::Circular})
 for(int n: {16,32,255}) for(int count: {1,2,3,4}){
  c.tft->ops.clear();host_heap_begin();{
   MultiTrend t(180,140,0,50,100,count,n,count,mode);t.setCanvas(&c);
   auto ctor=host_heap_stats();t.show();auto render=host_heap_stats();
   const size_t cache=(n*(count+1)+3*count)*sizeof(uint16_t);
   assert(render.requested-ctor.requested==cache && render.calls==ctor.calls+1);
   for(int j=0;j<count;++j)t.addValue(j,73,false);t.update();
   assert(host_heap_stats().calls==render.calls);
   std::printf("mode=%d S=%d N=%d ctor=%zu cache=%zu retained=%zu\n",int(mode),count,n,ctor.retained,cache,render.retained);
  }host_heap_end();
 }
 std::printf("sizeof Base=%zu Trend=%zu Multi=%zu; ring sample writes=1 per insertion\n",sizeof(TrendBase),sizeof(Trend),sizeof(MultiTrend));
}
