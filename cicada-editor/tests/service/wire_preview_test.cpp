// wire_preview_test.cpp — M1c：画线预览几何回归（= KiCad computeBreakPoint LINE_MODE_45
// 的 45° 分支 + lround 网格语义；吸附经 SnapEndpoints/SnapGrid，见引擎冒烟/curl 验证）。
// usage: cicada-engine-wiretest（断言失败 → 非零退出）
#include <service/wire_preview.h>

#include <stdio.h>
#include <string>

static int failures = 0;

#define RUN(name, cond)                                                          \
    do {                                                                         \
        if( !( cond ) )                                                          \
        {                                                                        \
            fprintf( stderr, "FAIL: %s (line %d)\n", name, __LINE__ );           \
            ++failures;                                                          \
        }                                                                        \
    } while( 0 )

int main()
{
    using cicada::editor::service::ComputeWireBreak;

    // !posture |dx| > |dy|: horizontal first, then 45° to the target (图1 shape)
    {
        auto out = ComputeWireBreak( { 0, 0 }, { 100, 50 }, std::nullopt, false );
        RUN( "45 h-first mid", out.mid.first == 50 && out.mid.second == 0 );
        RUN( "45 h-first end", out.end.first == 100 && out.end.second == 50 );
    }
    // !posture |dx| < |dy|: vertical first, then 45°
    {
        auto out = ComputeWireBreak( { 0, 0 }, { 50, 100 }, std::nullopt, false );
        RUN( "45 v-first mid", out.mid.first == 0 && out.mid.second == 50 );
        RUN( "45 v-first end", out.end.first == 50 && out.end.second == 100 );
    }
    // !posture pure vertical: mid == C (degenerate straight A→C)
    {
        auto out = ComputeWireBreak( { 10, 10 }, { 10, 60 }, std::nullopt, false );
        RUN( "45 pure-v mid", out.mid.first == 10 && out.mid.second == 60 );
        RUN( "45 pure-v end", out.end.first == 10 && out.end.second == 60 );
    }
    // posture horizontal prevDir: 45° lead, then horizontal tail
    {
        auto out = ComputeWireBreak( { 0, 0 }, { 100, 50 },
                                     std::optional<std::pair<int, int>>( { 100, 0 } ), true );
        RUN( "45 posture mid", out.mid.first == 50 && out.mid.second == 50 );
        RUN( "45 posture end", out.end.first == 100 && out.end.second == 50 );
    }
    // posture overshoot sanity: vertical preference for a flat cursor → fallback
    // to |dx| >= |dy| tie-break with the posture-horizontal formula (45° lead)
    {
        auto out = ComputeWireBreak( { 0, 0 }, { 100, 20 },
                                     std::optional<std::pair<int, int>>( { 0, 100 } ), true );
        RUN( "45 overshoot mid", out.mid.first == 20 && out.mid.second == 20 );
        RUN( "45 overshoot end", out.end.first == 100 && out.end.second == 20 );
    }
    // sign sanity across quadrants: dx/dy both negative
    {
        auto out = ComputeWireBreak( { 0, 0 }, { -100, -50 }, std::nullopt, false );
        RUN( "45 neg mid", out.mid.first == -50 && out.mid.second == 0 );
        RUN( "45 neg end", out.end.first == -100 && out.end.second == -50 );
    }

    if( failures )
    {
        fprintf( stderr, "wiretest: %d failure(s)\n", failures );
        return 1;
    }
    printf( "wiretest: OK\n" );
    return 0;
}
