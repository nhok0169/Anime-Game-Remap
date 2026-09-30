// ##### Credits

// ===== Anime Game Remap (AG Remap) =====
// Authors: Albert Gold#2696, NK#1321
//
// if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
// Special Thanks:
//   nguen#2011 (for support)
//   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
//   HazrateGolabi#1364 (for being awesome, and improving the code)

// ##### EndCredits

// -----------------------------------------------------------------------------
// Standalone test for AGRemapCore::InnerLayerOutline -- which vertices of a mesh's INNER layers draw no
// outline (Yaoyao5's long hair on YaoyaoBamboo, 2026-09-28).
//
// What is worth pinning here, because each was a real mistake before it was a rule:
//
//   * Only the COVERED layer goes: a sheet with another sheet along its normal loses its outline, and the
//     sheet on top keeps it -- that one draws the silhouette.
//   * The decision is per TRIANGLE, all three corners: two covered corners take the third with them. Decided
//     per vertex, a triangle kept one corner at the mod's width, its shell stretched into a wedge, and the
//     wedge showed in game as a dark rectangle on a front lock. ONE covered corner is not enough.
//   * A face turned IN towards the vertical axis through the targets' centre is inner with facingAxis, and
//     not without it -- a lock hanging in front of the shoulder has nothing along its body side's normal.
//   * A vertex outside the target triangles is never touched, covered or not (her eyes are in the same
//     head object and keep their outline).
//   * A triangle a vertex is a corner of does not cover it -- the ray starts ON that triangle.
//
// NOTE: nothing builds core/tests/*.cpp -- not CMake, not CI, not main.py. This file runs only when
// somebody compiles it; see VGComponentMerge_test.cpp's header for the command line.
// -----------------------------------------------------------------------------

#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <vector>

#include "AGRemapCore/model/buffers/InnerLayerOutline.h"

using AGRemapCore::InnerLayerOutline;
using Vec3 = InnerLayerOutline::Vec3;
using Triangles = InnerLayerOutline::Triangles;

namespace {
    int failures = 0;

    void expect(bool ok, const char* what) {
        if (!ok) {
            std::printf("FAIL: %s\n", what);
            ++failures;
        }
    }

    // A unit quad in the x/y plane at depth z, wound so its face points +z when `outward`
    void quad(std::vector<Vec3>& pos, std::vector<Vec3>& nrm, Triangles& tris, float x, float y, float z, float size, bool outward) {
        const unsigned long long base = pos.size();
        pos.push_back({x, y, z});
        pos.push_back({x + size, y, z});
        pos.push_back({x + size, y + size, z});
        pos.push_back({x, y + size, z});
        const float nz = outward ? 1.0f : -1.0f;
        for (int k = 0; k < 4; ++k) {
            nrm.push_back({0.0f, 0.0f, nz});
        }
        if (outward) {
            tris.push_back({base, base + 1, base + 2});
            tris.push_back({base, base + 2, base + 3});
        } else {
            tris.push_back({base, base + 2, base + 1});
            tris.push_back({base, base + 3, base + 2});
        }
    }

    bool all(const std::vector<bool>& v, std::size_t from, std::size_t to, bool want) {
        for (std::size_t i = from; i < to; ++i) {
            if (v[i] != want) {
                return false;
            }
        }
        return true;
    }
}

int main() {
    // ---- two sheets: the inner one covered by the outer, 5 cm along its normal ----
    {
        std::vector<Vec3> pos, nrm;
        Triangles inner, outer;
        quad(pos, nrm, inner, -0.5f, -0.5f, 0.0f, 1.0f, true);     // vertices 0-3
        quad(pos, nrm, outer, -0.6f, -0.6f, 0.05f, 1.2f, true);    // vertices 4-7, larger so every ray lands

        InnerLayerOutline rule;
        rule.facingAxis = false;
        const std::vector<bool> out = rule.find(pos, nrm, {&inner, &outer}, {&inner, &outer});
        expect(all(out, 0, 4, true), "the covered sheet loses its outline");
        expect(all(out, 4, 8, false), "the sheet on top keeps its outline");

        rule.reach = 0.03f;
        const std::vector<bool> near = rule.find(pos, nrm, {&inner, &outer}, {&inner, &outer});
        expect(all(near, 0, 8, false), "a layer further away than the reach does not cover");

        // targets exclude the inner sheet: never touched, even though covered
        rule.reach = 0.1f;
        const std::vector<bool> only = rule.find(pos, nrm, {&inner, &outer}, {&outer});
        expect(all(only, 0, 8, false), "a vertex outside the target triangles is never touched");
    }

    // ---- per TRIANGLE: two covered corners take the third; one does not ----
    {
        // One triangle (0, 1, 2) under a small occluder that covers only corners 0 and 1
        std::vector<Vec3> pos = {{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}};
        std::vector<Vec3> nrm(3, Vec3{0.0f, 0.0f, 1.0f});
        Triangles target = {{0, 1, 2}};
        Triangles occl;
        // a thin strip at z = 0.05 spanning x in [-0.1, 1.1], y in [-0.1, 0.1]: over corners 0 and 1 only
        const unsigned long long b = pos.size();
        pos.push_back({-0.1f, -0.1f, 0.05f}); pos.push_back({1.1f, -0.1f, 0.05f});
        pos.push_back({1.1f, 0.1f, 0.05f}); pos.push_back({-0.1f, 0.1f, 0.05f});
        for (int k = 0; k < 4; ++k) nrm.push_back({0.0f, 0.0f, 1.0f});
        occl.push_back({b, b + 1, b + 2});
        occl.push_back({b, b + 2, b + 3});

        InnerLayerOutline rule;
        rule.facingAxis = false;
        const std::vector<bool> cov = rule.covered(pos, nrm, {&target, &occl}, {&target});
        expect(cov[0] && cov[1] && !cov[2], "the strip covers corners 0 and 1 and not 2");
        const std::vector<bool> out = rule.find(pos, nrm, {&target, &occl}, {&target});
        expect(out[0] && out[1] && out[2], "two covered corners take the third corner with them (no wedge)");

        // Only corner 0 covered: a strip over x in [-0.1, 0.1]
        pos[b + 1] = {0.1f, -0.1f, 0.05f}; pos[b + 2] = {0.1f, 0.1f, 0.05f};
        const std::vector<bool> one = rule.find(pos, nrm, {&target, &occl}, {&target});
        expect(!one[0] && !one[1] && !one[2], "one covered corner is not enough");
    }

    // ---- a triangle a vertex belongs to does not cover it ----
    {
        // A fold: vertex 0's normal runs straight along its own second triangle
        std::vector<Vec3> pos = {{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
                                 {0.0f, 0.0f, 0.05f}, {1.0f, 0.0f, 0.05f}};
        std::vector<Vec3> nrm(5, Vec3{0.0f, 0.0f, 1.0f});
        Triangles tris = {{0, 1, 2}, {0, 3, 4}};
        InnerLayerOutline rule;
        rule.facingAxis = false;
        const std::vector<bool> cov = rule.covered(pos, nrm, {&tris}, {&tris});
        expect(!cov[0], "a vertex is not covered by a triangle it is a corner of");
    }

    // ---- facing the axis: a sheet turned in towards the targets' centre ----
    {
        // Two sheets in y/z planes at x = +1 and x = -1, both with faces pointing +x: the one at x = +1 faces
        // OUT, the one at x = -1 faces IN (towards the centre, x = 0)
        std::vector<Vec3> pos, nrm;
        Triangles right, left;
        auto side = [&](Triangles& tris, float x) {
            const unsigned long long base = pos.size();
            pos.push_back({x, 0.0f, 0.0f}); pos.push_back({x, 1.0f, 0.0f});
            pos.push_back({x, 1.0f, 1.0f}); pos.push_back({x, 0.0f, 1.0f});
            for (int k = 0; k < 4; ++k) nrm.push_back({1.0f, 0.0f, 0.0f});
            // face normal +x: (p1 - p0) x (p2 - p0) = (0,1,0) x (0,1,1) = (1, 0, 0)
            tris.push_back({base, base + 1, base + 2});
            tris.push_back({base, base + 2, base + 3});
        };
        side(right, 1.0f);     // 0-3
        side(left, -1.0f);     // 4-7

        InnerLayerOutline rule;
        rule.facingAxis = true;
        const std::vector<bool> out = rule.find(pos, nrm, {&right, &left}, {&right, &left});
        expect(all(out, 0, 4, false), "a sheet facing out from the axis keeps its outline");
        expect(all(out, 4, 8, true), "a sheet facing in towards the axis loses its outline");

        rule.facingAxis = false;
        const std::vector<bool> off = rule.find(pos, nrm, {&right, &left}, {&right, &left});
        expect(all(off, 0, 8, false), "without facingAxis, facing in is not enough");
    }

    // ---- backed: a layer facing the OTHER way right behind a triangle (VGComponentSpec::mirrorBackedReach) ----
    {
        std::vector<Vec3> pos, nrm;
        Triangles coat, lining, body;
        quad(pos, nrm, coat, 0.0f, 0.0f, 0.0f, 1.0f, true);          // 0-3, facing +z
        quad(pos, nrm, lining, 0.0f, 0.0f, -0.004f, 1.0f, false);    // 4-7, 4 mm behind, facing -z
        quad(pos, nrm, body, 3.0f, 0.0f, -0.004f, 1.0f, true);       // 8-11, under the NEXT panel, facing +z
        Triangles panel;
        quad(pos, nrm, panel, 3.0f, 0.0f, 0.0f, 1.0f, true);         // 12-15, over `body`, same way

        const std::vector<const Triangles*> all = {&coat, &lining, &body, &panel};
        const std::vector<bool> c = InnerLayerOutline::backed(pos, nrm, all, coat, 0.01f);
        expect(c.size() == 2 && c[0] && c[1], "a coat with its lining 4 mm behind is backed");
        const std::vector<bool> l = InnerLayerOutline::backed(pos, nrm, all, lining, 0.01f);
        expect(l[0] && l[1], "and so is the lining, by the coat");
        const std::vector<bool> p = InnerLayerOutline::backed(pos, nrm, all, panel, 0.01f);
        expect(!p[0] && !p[1], "a layer facing the SAME way behind (a body under cloth) does not back it");
        const std::vector<bool> far = InnerLayerOutline::backed(pos, nrm, all, coat, 0.003f);
        expect(!far[0] && !far[1], "a lining further away than the reach does not back it");
        const std::vector<bool> none = InnerLayerOutline::backed(pos, nrm, all, coat, 0.0f);
        expect(!none[0] && !none[1], "a reach of 0 backs nothing");
        const std::vector<bool> self = InnerLayerOutline::backed(pos, nrm, {&coat}, coat, 0.01f);
        expect(!self[0] && !self[1], "a triangle does not back itself, nor its neighbour in the same plane");
    }

    // ---- backed: a lining under only a CORNER of the triangle (the centroid's ray misses it) ----
    {
        std::vector<Vec3> pos = {{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}};
        std::vector<Vec3> nrm(3, Vec3{0.0f, 0.0f, 1.0f});
        Triangles target = {{0, 1, 2}};
        // a small patch facing -z, 4 mm under corner 1 only: it holds the point a tenth of the way from corner 1 to
        // the centroid, (0.933, 0.033), and not the centroid (0.333, 0.333)
        Triangles patch;
        const unsigned long long b = pos.size();
        pos.push_back({0.8f, -0.1f, -0.004f}); pos.push_back({0.8f, 0.3f, -0.004f}); pos.push_back({1.3f, -0.1f, -0.004f});
        for (int k = 0; k < 3; ++k) nrm.push_back({0.0f, 0.0f, -1.0f});
        patch.push_back({b, b + 1, b + 2});     // (p1 - p0) x (p2 - p0) = (0, .4, 0) x (.5, 0, 0) -> -z
        const std::vector<bool> r = InnerLayerOutline::backed(pos, nrm, {&target, &patch}, target, 0.01f);
        expect(r[0], "a lining under one corner backs the triangle -- its twin's corner would come through it");
    }

    // ---- reading a Position.buf ----
    {
        AGRemapCore::ByteVec buf(40 * 2, 0);
        const float a[10] = {1, 2, 3, 0, 0, 1, 0, 0, 1, -1};
        const float b[10] = {4, 5, 6, 1, 0, 0, 1, 0, 0, -1};
        std::memcpy(buf.data(), a, 40);
        std::memcpy(buf.data() + 40, b, 40);
        std::vector<Vec3> pos, nrm;
        InnerLayerOutline::readPositions(buf, 40, pos, nrm);
        expect(pos.size() == 2 && pos[1][0] == 4.0f && nrm[0][2] == 1.0f && nrm[1][0] == 1.0f, "positions and normals read");

        bool threw = false;
        try {
            InnerLayerOutline::readPositions(buf, 12, pos, nrm);
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        expect(threw, "a stride with no normal is refused");
    }

    if (failures == 0) {
        std::printf("InnerLayerOutline_test: all passed\n");
        return 0;
    }
    std::printf("InnerLayerOutline_test: %d failed\n", failures);
    return 1;
}
