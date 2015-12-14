#include <glib.h>

#include <clocale>
#include <vector>
#include <string>
#include <map>
#include <limits>

#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <iostream>

#include "lensfun.h"
#include "../libs/lensfun/lensfunprv.h"


typedef struct
{
  void       *coordBuff;
  size_t      img_width, img_height;
  lfLens     *lens;
  lfModifier *mod;
} lfFixture;

// setup a standard lens
void mod_setup (lfFixture *lfFix, gconstpointer data)
{
    lfFix->lens             = new lfLens();
    lfFix->lens->CropFactor = 1.534f;
    lfFix->lens->AspectRatio = 1.5f;
    lfFix->lens->Type       = LF_RECTILINEAR;

    lfFix->img_height = 1000;
    lfFix->img_width  = 1500;

    lfFix->mod = lfModifier::Create (lfFix->lens, 1.534f, lfFix->img_width, lfFix->img_height);

    lfFix->mod->Initialize(lfFix->lens, LF_PF_F32, 50.89f, 2.8f, 1000.0f, 1.0f, LF_RECTILINEAR,
                           0, false);

    lfFix->coordBuff = NULL;

    const size_t bufsize = 2 * lfFix->img_width * lfFix->img_height * sizeof (float);
    lfFix->coordBuff = g_malloc (bufsize);
}

// setup a standard image in portrait mode
void mod_setup_portrait (lfFixture *lfFix, gconstpointer data)
{
    lfFix->lens             = new lfLens();
    lfFix->lens->CropFactor = 1.534f;
    lfFix->lens->AspectRatio = 1.5f;
    lfFix->lens->Type       = LF_RECTILINEAR;

    lfFix->img_height = 1500;
    lfFix->img_width  = 1000;

    lfFix->mod = lfModifier::Create (lfFix->lens, 1.534f, lfFix->img_width, lfFix->img_height);

    lfFix->mod->Initialize(lfFix->lens, LF_PF_F32, 50.89f, 2.8f, 1000.0f, 1.0f, LF_RECTILINEAR,
                           0, false);

    lfFix->coordBuff = NULL;

    const size_t bufsize = 2 * lfFix->img_width * lfFix->img_height * sizeof (float);
    lfFix->coordBuff = g_malloc (bufsize);
}

void mod_teardown (lfFixture *lfFix, gconstpointer data)
{
    g_free (lfFix->coordBuff);

    lfFix->mod->Destroy();
    delete lfFix->lens;
}

void test_mod_coord_pc_svd (lfFixture *lfFix, gconstpointer data)
{
    fvector x (5), y (5);
    x[0] = 1; y[0] = 1;
    x[1] = 2; y[1] = 2;
    x[2] = 3; y[2] = 2;
    x[3] = 2; y[3] = 0;
    x[4] = 1; y[4] = 1.5;
    matrix M (5, fvector (6));
    for (int i = 0; i < 5; i++)
    {
        M [i][0] = x [i] * x [i];
        M [i][1] = x [i] * y [i];
        M [i][2] = y [i] * y [i];
        M [i][3] = x [i];
        M [i][4] = y [i];
        M [i][5] = 1;
    }
    fvector result = svd (M);
    const float epsilon = std::numeric_limits<float>::epsilon() * 20;
    g_assert_cmpfloat (fabs (result [0] - 0.0475653), <=, epsilon);
    g_assert_cmpfloat (fabs (result [1] - 0.0951303), <=, epsilon);
    g_assert_cmpfloat (fabs (result [2] - 0.190261), <=, epsilon);
    g_assert_cmpfloat (fabs (result [3] + 0.428087), <=, epsilon);
    g_assert_cmpfloat (fabs (result [4] + 0.570781), <=, epsilon);
    g_assert_cmpfloat (fabs (result [5] - 0.665912), <=, epsilon);
}

void test_mod_coord_pc_4_points (lfFixture *lfFix, gconstpointer data)
{
    // Bases on DSC02275.json
    const float epsilon = std::numeric_limits<float>::epsilon();

    float temp_x[] = {503, 1063, 509, 1066};
    float temp_y[] = {150, 197, 860, 759};
    fvector x (temp_x, temp_x + 4);
    fvector y (temp_y, temp_y + 4);
    g_assert_true (lfFix->mod->enable_perspective_correction (x, y, 0));

    float expected_x[] = {194.43689f, 283.340454f, 366.920929f, 445.64209f, 519.91571f,
                          590.108704f, 656.548218f, 719.527649f, 779.31012f, 836.133301f};
    float expected_y[] = {-88.5747986f, 45.5082512f, 171.562714f, 290.288757f, 402.307434f,
                          508.171478f, 608.374634f, 703.359009f, 793.522034f, 879.221741f};
    fvector coords (2);
    for (int i = 0; i < 10; i++)
    {
        g_assert_true (lfFix->mod->ApplyGeometryDistortion (100.0f * i, 100.0f * i, 1, 1, &coords [0]));
        g_assert_cmpfloat (fabs (coords [0] - expected_x [i]), <=, epsilon);
        g_assert_cmpfloat (fabs (coords [1] - expected_y [i]), <=, epsilon);
    }
}

void test_mod_coord_pc_4_points_portrait (lfFixture *lfFix, gconstpointer data)
{
    // Bases on DSC02277.json
    const float epsilon = std::numeric_limits<float>::epsilon();

    float temp_x[] = {145, 208, 748, 850};
    float temp_y[] = {1060, 666, 668, 1060};
    fvector x (temp_x, temp_x + 4);
    fvector y (temp_y, temp_y + 4);
    g_assert_true (lfFix->mod->enable_perspective_correction (x, y, 0));

    float expected_x[] = {71.1176453f, 147.857544f, 228.261032f, 312.596893f, 401.160767f,
                          494.278839f, 592.311707f, 695.658936f, 804.764648f, 920.124146f};
    float expected_y[] = {508.424408f, 535.728027f, 564.335327f, 594.341736f, 625.852295f,
                          658.983337f, 693.863159f, 730.633606f, 769.452942f, 810.497437f};
    fvector coords (2);
    for (int i = 0; i < 10; i++)
    {
        g_assert_true (lfFix->mod->ApplyGeometryDistortion (100.0f * i, 100.0f * i, 1, 1, &coords [0]));
        g_assert_cmpfloat (fabs (coords [0] - expected_x [i]), <=, epsilon);
        g_assert_cmpfloat (fabs (coords [1] - expected_y [i]), <=, epsilon);
    }
}

void test_mod_coord_pc_8_points (lfFixture *lfFix, gconstpointer data)
{
    // Bases on DSC02278.json
    const float epsilon = std::numeric_limits<float>::epsilon();

    float temp_x[] = {615, 264, 1280, 813, 615, 1280, 264, 813};
    float temp_y[] = {755, 292, 622, 220, 755, 622, 292, 220};
    fvector x (temp_x, temp_x + 8);
    fvector y (temp_y, temp_y + 8);
    g_assert_true (lfFix->mod->enable_perspective_correction (x, y, 0));

    float expected_x[] = {-112.681725f, 7.11962175f, 129.830688f, 255.558914f, 384.416901f,
                          516.522705f, 652.001343f, 790.982971f, 933.605408f, 1080.01367f};
    float expected_y[] = {394.968201f, 422.388306f, 450.474274f, 479.251129f, 508.743988f,
                          538.980408f, 569.988647f, 601.798706f, 634.442017f, 667.951965f};
    fvector coords (2);
    for (int i = 0; i < 10; i++)
    {
        g_assert_true (lfFix->mod->ApplyGeometryDistortion (100.0f * i, 100.0f * i, 1, 1, &coords [0]));
        g_assert_cmpfloat (fabs (coords [0] - expected_x [i]), <=, epsilon);
        g_assert_cmpfloat (fabs (coords [1] - expected_y [i]), <=, epsilon);
    }
}

void test_mod_coord_pc_0_points (lfFixture *lfFix, gconstpointer data)
{
    // Bases on DSC02279.json
    const float epsilon = std::numeric_limits<float>::epsilon();

    fvector empty_list;
    g_assert_false (lfFix->mod->enable_perspective_correction (empty_list, empty_list, 0));

    fvector coords (2);
    for (int i = 0; i < 10; i++)
    {
        g_assert_false (lfFix->mod->ApplyGeometryDistortion (100.0f * i, 100.0f * i, 1, 1, &coords [0]));
        g_assert_cmpfloat (fabs (coords [0]), <=, epsilon);
        g_assert_cmpfloat (fabs (coords [1]), <=, epsilon);
    }
}


int main (int argc, char **argv)
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);

  g_test_add ("/modifier/coord/pc/svd", lfFixture, NULL,
              mod_setup, test_mod_coord_pc_svd, mod_teardown);
  g_test_add ("/modifier/coord/pc/4 points", lfFixture, NULL,
              mod_setup, test_mod_coord_pc_4_points, mod_teardown);
  g_test_add ("/modifier/coord/pc/4 points portrait", lfFixture, NULL,
              mod_setup_portrait, test_mod_coord_pc_4_points_portrait, mod_teardown);
  g_test_add ("/modifier/coord/pc/8 points", lfFixture, NULL,
              mod_setup, test_mod_coord_pc_8_points, mod_teardown);
  g_test_add ("/modifier/coord/pc/no points", lfFixture, NULL,
              mod_setup, test_mod_coord_pc_0_points, mod_teardown);

  return g_test_run();
}