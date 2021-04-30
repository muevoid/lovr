#include "api/api.h"
#include "graphics/graphics.h"
#include "core/maf.h"
#include "core/util.h"
#include <lua.h>
#include <lauxlib.h>
#include <string.h>

static int l_lovrCanvasBegin(lua_State* L) {
  Canvas* canvas = luax_checktype(L, 1, Canvas);
  lovrCanvasBegin(canvas);
  return 0;
}

static int l_lovrCanvasFinish(lua_State* L) {
  Canvas* canvas = luax_checktype(L, 1, Canvas);
  lovrCanvasFinish(canvas);
  return 0;
}

static int l_lovrCanvasIsActive(lua_State* L) {
  Canvas* canvas = luax_checktype(L, 1, Canvas);
  lua_pushboolean(L, lovrCanvasIsActive(canvas));
  return 1;
}

static int l_lovrCanvasGetAlphaToCoverage(lua_State* L) {
  Canvas* canvas = luax_checktype(L, 1, Canvas);
  lua_pushboolean(L, lovrCanvasGetAlphaToCoverage(canvas));
  return 1;
}

static int l_lovrCanvasSetAlphaToCoverage(lua_State* L) {
  Canvas* canvas = luax_checktype(L, 1, Canvas);
  lovrCanvasSetAlphaToCoverage(canvas, lua_toboolean(L, 2));
  return 1;
}

static int l_lovrCanvasGetBlendMode(lua_State* L) {
  Canvas* canvas = luax_checktype(L, 1, Canvas);
  uint32_t target = luaL_optinteger(L, 2, 1) - 1;
  lovrAssert(target < 4, "Invalid color target index: %d", target + 1);
  BlendMode mode;
  BlendAlphaMode alphaMode;
  lovrCanvasGetBlendMode(canvas, target, &mode, &alphaMode);
  if (mode == BLEND_NONE) {
    lua_pushnil(L);
    return 1;
  } else {
    luax_pushenum(L, BlendMode, mode);
    luax_pushenum(L, BlendAlphaMode, alphaMode);
    return 2;
  }
}

static int l_lovrCanvasSetBlendMode(lua_State* L) {
  Canvas* canvas = luax_checktype(L, 1, Canvas);
  if (lua_type(L, 2) == LUA_TNUMBER) {
    uint32_t target = lua_tonumber(L, 2) - 1;
    lovrAssert(target < 4, "Invalid color target index: %d", target + 1);
    BlendMode mode = lua_isnoneornil(L, 3) ? BLEND_NONE : luax_checkenum(L, 3, BlendMode, NULL);
    BlendAlphaMode alphaMode = luax_checkenum(L, 4, BlendAlphaMode, "alphamultiply");
    lovrCanvasSetBlendMode(canvas, target, mode, alphaMode);
    return 0;
  }

  BlendMode mode = lua_isnoneornil(L, 2) ? BLEND_NONE : luax_checkenum(L, 2, BlendMode, NULL);
  BlendAlphaMode alphaMode = luax_checkenum(L, 3, BlendAlphaMode, "alphamultiply");
  for (uint32_t i = 0; i < 4; i++) {
    lovrCanvasSetBlendMode(canvas, i, mode, alphaMode);
  }
  return 0;
}

static int l_lovrCanvasGetColorMask(lua_State* L) {
  Canvas* canvas = luax_checktype(L, 1, Canvas);
  uint32_t target = luaL_optinteger(L, 2, 1) - 1;
  lovrAssert(target < 4, "Invalid color target index: %d", target + 1);
  bool r, g, b, a;
  lovrCanvasGetColorMask(canvas, target, &r, &g, &b, &a);
  lua_pushboolean(L, r);
  lua_pushboolean(L, g);
  lua_pushboolean(L, b);
  lua_pushboolean(L, a);
  return 4;
}

static int l_lovrCanvasSetColorMask(lua_State* L) {
  Canvas* canvas = luax_checktype(L, 1, Canvas);
  if (lua_type(L, 2) == LUA_TNUMBER) {
    uint32_t target = lua_tonumber(L, 2) - 1;
    lovrAssert(target < 4, "Invalid color target index: %d", target + 1);
    bool r = lua_toboolean(L, 3);
    bool g = lua_toboolean(L, 4);
    bool b = lua_toboolean(L, 5);
    bool a = lua_toboolean(L, 6);
    lovrCanvasSetColorMask(canvas, target, r, g, b, a);
    return 0;
  }

  bool r = lua_toboolean(L, 2);
  bool g = lua_toboolean(L, 3);
  bool b = lua_toboolean(L, 4);
  bool a = lua_toboolean(L, 5);
  for (uint32_t i = 0; i < 4; i++) {
    lovrCanvasSetColorMask(canvas, i, r, g, b, a);
  }
  return 0;
}

static int l_lovrCanvasGetCullMode(lua_State* L) {
  Canvas* canvas = luax_checktype(L, 1, Canvas);
  luax_pushenum(L, CullMode, lovrCanvasGetCullMode(canvas));
  return 1;
}

static int l_lovrCanvasSetCullMode(lua_State* L) {
  Canvas* canvas = luax_checktype(L, 1, Canvas);
  CullMode mode = luax_checkenum(L, 2, CullMode, "none");
  lovrCanvasSetCullMode(canvas, mode);
  return 0;
}

static int l_lovrCanvasGetDepthTest(lua_State* L) {
  Canvas* canvas = luax_checktype(L, 1, Canvas);
  CompareMode test;
  bool write;
  lovrCanvasGetDepthTest(canvas, &test, &write);
  if (test == COMPARE_NONE) {
    lua_pushnil(L);
  } else {
    luax_pushenum(L, CompareMode, test);
  }
  lua_pushboolean(L, write);
  return 2;
}

static int l_lovrCanvasSetDepthTest(lua_State* L) {
  Canvas* canvas = luax_checktype(L, 1, Canvas);
  CompareMode test = lua_isnoneornil(L, 2) ? COMPARE_NONE : luax_checkenum(L, 2, CompareMode, NULL);
  bool write = lua_isnoneornil(L, 3) ? true : lua_toboolean(L, 3);
  lovrCanvasSetDepthTest(canvas, test, write);
  return 0;
}

static int l_lovrCanvasGetDepthNudge(lua_State* L) {
  Canvas* canvas = luax_checktype(L, 1, Canvas);
  float nudge, sloped, clamp;
  lovrCanvasGetDepthNudge(canvas, &nudge, &sloped, &clamp);
  lua_pushnumber(L, nudge);
  lua_pushnumber(L, sloped);
  lua_pushnumber(L, clamp);
  return 3;
}

static int l_lovrCanvasSetDepthNudge(lua_State* L) {
  Canvas* canvas = luax_checktype(L, 1, Canvas);
  float nudge = luax_optfloat(L, 2, 0.f);
  float sloped = luax_optfloat(L, 3, 0.f);
  float clamp = luax_optfloat(L, 4, 0.f);
  lovrCanvasSetDepthNudge(canvas, nudge, sloped, clamp);
  return 0;
}

static int l_lovrCanvasGetDepthClamp(lua_State* L) {
  Canvas* canvas = luax_checktype(L, 1, Canvas);
  bool clamp = lovrCanvasGetDepthClamp(canvas);
  lua_pushboolean(L, clamp);
  return 1;
}

static int l_lovrCanvasSetDepthClamp(lua_State* L) {
  Canvas* canvas = luax_checktype(L, 1, Canvas);
  bool clamp = lua_toboolean(L, 2);
  lovrCanvasSetDepthClamp(canvas, clamp);
  return 0;
}

static int l_lovrCanvasGetShader(lua_State* L) {
  Canvas* canvas = luax_checktype(L, 1, Canvas);
  Shader* shader = lovrCanvasGetShader(canvas);
  luax_pushtype(L, Shader, shader);
  return 1;
}

static int l_lovrCanvasSetShader(lua_State* L) {
  Canvas* canvas = luax_checktype(L, 1, Canvas);
  Shader* shader = lua_isnoneornil(L, 2) ? NULL : luax_checktype(L, 2, Shader);
  lovrCanvasSetShader(canvas, shader);
  return 0;
}

static int l_lovrCanvasGetStencilTest(lua_State* L) {
  Canvas* canvas = luax_checktype(L, 1, Canvas);
  CompareMode test;
  uint8_t value;
  lovrCanvasGetStencilTest(canvas, &test, &value);
  if (test == COMPARE_NONE) {
    lua_pushnil(L);
    return 1;
  }
  luax_pushenum(L, CompareMode, test);
  lua_pushinteger(L, value);
  return 2;
}

static int l_lovrCanvasSetStencilTest(lua_State* L) {
  Canvas* canvas = luax_checktype(L, 1, Canvas);
  if (lua_isnoneornil(L, 2)) {
    lovrCanvasSetStencilTest(canvas, COMPARE_NONE, 0);
  } else {
    CompareMode test = luax_checkenum(L, 2, CompareMode, NULL);
    uint8_t value = luaL_checkinteger(L, 3);
    lovrCanvasSetStencilTest(canvas, test, value);
  }
  return 0;
}

static int l_lovrCanvasGetWinding(lua_State* L) {
  Canvas* canvas = luax_checktype(L, 1, Canvas);
  Winding winding = lovrCanvasGetWinding(canvas);
  luax_pushenum(L, Winding, winding);
  return 1;
}

static int l_lovrCanvasSetWinding(lua_State* L) {
  Canvas* canvas = luax_checktype(L, 1, Canvas);
  Winding winding = luax_checkenum(L, 2, Winding, NULL);
  lovrCanvasSetWinding(canvas, winding);
  return 0;
}

static int l_lovrCanvasIsWireframe(lua_State* L) {
  Canvas* canvas = luax_checktype(L, 1, Canvas);
  bool wireframe = lovrCanvasIsWireframe(canvas);
  lua_pushboolean(L, wireframe);
  return 1;
}

static int l_lovrCanvasSetWireframe(lua_State* L) {
  Canvas* canvas = luax_checktype(L, 1, Canvas);
  bool wireframe = lua_toboolean(L, 2);
  lovrCanvasSetWireframe(canvas, wireframe);
  return 0;
}

static int l_lovrCanvasPush(lua_State* L) {
  Canvas* canvas = luax_checktype(L, 1, Canvas);
  lovrCanvasPush(canvas);
  return 0;
}

static int l_lovrCanvasPop(lua_State* L) {
  Canvas* canvas = luax_checktype(L, 1, Canvas);
  lovrCanvasPop(canvas);
  return 0;
}

static int l_lovrCanvasOrigin(lua_State* L) {
  Canvas* canvas = luax_checktype(L, 1, Canvas);
  lovrCanvasOrigin(canvas);
  return 0;
}

static int l_lovrCanvasTranslate(lua_State* L) {
  Canvas* canvas = luax_checktype(L, 1, Canvas);
  float translation[4];
  luax_readvec3(L, 2, translation, NULL);
  lovrCanvasTranslate(canvas, translation);
  return 0;
}

static int l_lovrCanvasRotate(lua_State* L) {
  Canvas* canvas = luax_checktype(L, 1, Canvas);
  float rotation[4];
  luax_readquat(L, 2, rotation, NULL);
  lovrCanvasRotate(canvas, rotation);
  return 0;
}

static int l_lovrCanvasScale(lua_State* L) {
  Canvas* canvas = luax_checktype(L, 1, Canvas);
  float scale[4];
  luax_readscale(L, 2, scale, 3, NULL);
  lovrCanvasScale(canvas, scale);
  return 0;
}

static int l_lovrCanvasTransform(lua_State* L) {
  Canvas* canvas = luax_checktype(L, 1, Canvas);
  float transform[16];
  luax_readmat4(L, 2, transform, 3);
  lovrCanvasTransform(canvas, transform);
  return 0;
}

static int l_lovrCanvasGetViewPose(lua_State* L) {
  Canvas* canvas = luax_checktype(L, 1, Canvas);
  uint32_t view = luaL_checkinteger(L, 2) - 1;
  lovrAssert(view < 6, "Invalid view index %d", view + 1);
  if (lua_gettop(L) > 2) {
    float* matrix = luax_checkvector(L, 3, V_MAT4, NULL);
    bool invert = lua_toboolean(L, 4);
    lovrCanvasGetViewMatrix(canvas, view, matrix);
    if (!invert) mat4_invert(matrix);
    lua_settop(L, 3);
    return 1;
  } else {
    float matrix[16], angle, ax, ay, az;
    lovrCanvasGetViewMatrix(canvas, view, matrix);
    mat4_invert(matrix);
    mat4_getAngleAxis(matrix, &angle, &ax, &ay, &az);
    lua_pushnumber(L, matrix[12]);
    lua_pushnumber(L, matrix[13]);
    lua_pushnumber(L, matrix[14]);
    lua_pushnumber(L, angle);
    lua_pushnumber(L, ax);
    lua_pushnumber(L, ay);
    lua_pushnumber(L, az);
    return 7;
  }
}

static int l_lovrCanvasSetViewPose(lua_State* L) {
  Canvas* canvas = luax_checktype(L, 1, Canvas);
  uint32_t view = luaL_checkinteger(L, 2) - 1;
  lovrAssert(view < 6, "Invalid view index %d", view + 1);
  VectorType type;
  float* p = luax_tovector(L, 3, &type);
  if (p && type == V_MAT4) {
    float matrix[16];
    mat4_init(matrix, p);
    bool inverted = lua_toboolean(L, 3);
    if (!inverted) mat4_invert(matrix);
    lovrCanvasSetViewMatrix(canvas, view, matrix);
  } else {
    int index = 3;
    float position[4], orientation[4], matrix[16];
    index = luax_readvec3(L, index, position, "vec3, number, or mat4");
    index = luax_readquat(L, index, orientation, NULL);
    mat4_fromQuat(matrix, orientation);
    memcpy(matrix + 12, position, 3 * sizeof(float));
    mat4_invert(matrix);
    lovrCanvasSetViewMatrix(canvas, view, matrix);
  }
  return 0;
}

static int l_lovrCanvasGetProjection(lua_State* L) {
  Canvas* canvas = luax_checktype(L, 1, Canvas);
  uint32_t view = luaL_checkinteger(L, 2) - 1;
  lovrAssert(view < 6, "Invalid view index %d", view + 1);
  if (lua_gettop(L) > 2) {
    float* matrix = luax_checkvector(L, 3, V_MAT4, NULL);
    lovrCanvasGetProjection(canvas, view, matrix);
    lua_settop(L, 3);
    return 1;
  } else {
    float matrix[16], left, right, up, down;
    lovrCanvasGetProjection(canvas, view, matrix);
    mat4_getFov(matrix, &left, &right, &up, &down);
    lua_pushnumber(L, left);
    lua_pushnumber(L, right);
    lua_pushnumber(L, up);
    lua_pushnumber(L, down);
    return 4;
  }
}

static int l_lovrCanvasSetProjection(lua_State* L) {
  Canvas* canvas = luax_checktype(L, 1, Canvas);
  uint32_t view = luaL_checkinteger(L, 2) - 1;
  lovrAssert(view < 6, "Invalid view index %d", view + 1);
  if (lua_type(L, 3) == LUA_TNUMBER) {
    float left = luax_checkfloat(L, 3);
    float right = luax_checkfloat(L, 4);
    float up = luax_checkfloat(L, 5);
    float down = luax_checkfloat(L, 6);
    float clipNear = luax_optfloat(L, 7, .1f);
    float clipFar = luax_optfloat(L, 8, 100.f);
    float matrix[16];
    mat4_fov(matrix, left, right, up, down, clipNear, clipFar);
    lovrCanvasSetProjection(canvas, view, matrix);
  } else {
    float* matrix = luax_checkvector(L, 2, V_MAT4, "mat4 or number");
    lovrCanvasSetProjection(canvas, view, matrix);
  }
  return 0;
}

static void onStencil(void* userdata) {
  lua_State* L = userdata;
  luaL_checktype(L, -1, LUA_TFUNCTION);
  lua_call(L, 0, 0);
}

static int l_lovrCanvasStencil(lua_State* L) {
  Canvas* canvas = luax_checktype(L, 1, Canvas);
  luaL_checktype(L, 2, LUA_TFUNCTION);
  StencilAction action = luax_checkenum(L, 3, StencilAction, "replace");
  uint8_t value = luaL_optinteger(L, 4, 1);
  StencilAction depthAction = luax_checkenum(L, 5, StencilAction, "keep");
  lovrCanvasStencil(canvas, action, depthAction, value, onStencil, L);
  return 0;
}

const luaL_Reg lovrCanvas[] = {
  { "begin", l_lovrCanvasBegin },
  { "finish", l_lovrCanvasFinish },
  { "isActive", l_lovrCanvasIsActive },
  { "getAlphaToCoverage", l_lovrCanvasGetAlphaToCoverage },
  { "setAlphaToCoverage", l_lovrCanvasSetAlphaToCoverage },
  { "getBlendMode", l_lovrCanvasGetBlendMode },
  { "setBlendMode", l_lovrCanvasSetBlendMode },
  { "getColorMask", l_lovrCanvasGetColorMask },
  { "setColorMask", l_lovrCanvasSetColorMask },
  { "getCullMode", l_lovrCanvasGetCullMode },
  { "setCullMode", l_lovrCanvasSetCullMode },
  { "getDepthTest", l_lovrCanvasGetDepthTest },
  { "setDepthTest", l_lovrCanvasSetDepthTest },
  { "getDepthNudge", l_lovrCanvasGetDepthNudge },
  { "setDepthNudge", l_lovrCanvasSetDepthNudge },
  { "getDepthClamp", l_lovrCanvasGetDepthClamp },
  { "setDepthClamp", l_lovrCanvasSetDepthClamp },
  { "getShader", l_lovrCanvasGetShader },
  { "setShader", l_lovrCanvasSetShader },
  { "getStencilTest", l_lovrCanvasGetStencilTest },
  { "setStencilTest", l_lovrCanvasSetStencilTest },
  { "getWinding", l_lovrCanvasGetWinding },
  { "setWinding", l_lovrCanvasSetWinding },
  { "isWireframe", l_lovrCanvasIsWireframe },
  { "setWireframe", l_lovrCanvasSetWireframe },
  { "push", l_lovrCanvasPush },
  { "pop", l_lovrCanvasPop },
  { "origin", l_lovrCanvasOrigin },
  { "translate", l_lovrCanvasTranslate },
  { "rotate", l_lovrCanvasRotate },
  { "scale", l_lovrCanvasScale },
  { "transform", l_lovrCanvasTransform },
  { "getViewPose", l_lovrCanvasGetViewPose },
  { "setViewPose", l_lovrCanvasSetViewPose },
  { "getProjection", l_lovrCanvasGetProjection },
  { "setProjection", l_lovrCanvasSetProjection },
  { "stencil", l_lovrCanvasStencil },
  { NULL, NULL }
};
