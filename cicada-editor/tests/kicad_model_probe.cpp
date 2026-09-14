// Compile-only K1b probe.  Do not include this file in the production editor
// until the KiCad dependency manifest and licensing boundary are accepted.
// The KiCad checkout is a read-only source reference; this translation unit
// intentionally has no runtime entry point and no KiCad link dependency.

#include <sch_item.h>
#include <sch_line.h>
#include <sch_junction.h>
#include <sch_no_connect.h>
#include <sch_label.h>
#include <sch_screen.h>

#include <type_traits>

static_assert(std::is_base_of_v<SCH_ITEM, SCH_LINE>);
static_assert(std::is_base_of_v<SCH_ITEM, SCH_JUNCTION>);
static_assert(std::is_base_of_v<SCH_ITEM, SCH_NO_CONNECT>);

void cicada_kicad_model_header_probe()
{
    // Exercise only declarations needed by the future adapter.  No object is
    // allowed to escape this function and no global KiCad/settings state is
    // touched.  Linking/ownership behavior is tested in a later probe stage.
    SCH_LINE wire;
    wire.SetStartPoint(VECTOR2I(0, 0));
    wire.SetEndPoint(VECTOR2I(100, 0));
    (void)wire.GetConnectionPoints();

    SCH_JUNCTION junction(VECTOR2I(100, 0));
    (void)junction.GetConnectionPoints();

    SCH_NO_CONNECT no_connect(VECTOR2I(200, 0));
    (void)no_connect.GetConnectionPoints();

    SCH_SCREEN* screen = nullptr;
    (void)screen;
}
