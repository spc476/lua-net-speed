
#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>

#define TYPE_BUFFER	"buffer"

typedef struct
{
  size_t  len;
  size_t  idx;
  char   *buf;
} buffer__s;

static int buffermeta___gc(lua_State *L)
{
  buffer__s *buf = luaL_checkudata(L,1,TYPE_BUFFER);
  free(buf->buf);
  return 0;
}

static int buffermeta___len(lua_State *L)
{
  buffer__s *buf = luaL_checkudata(L,1,TYPE_BUFFER);
  lua_pushinteger(L,buf->idx);
  return 1;
}

static int buffermeta___tostring(lua_State *L)
{
  buffer__s *buf = luaL_checkudata(L,1,TYPE_BUFFER);
  lua_pushlstring(L,buf->buf,buf->idx);
  return 1;
}

static ptrdiff_t posrelat(ptrdiff_t pos,size_t len)
{
  if (pos < 0) pos += (ptrdiff_t)len + 1;
  return (pos >= 0) ? pos : 0;
}

static int buffermeta_byte(lua_State *L)
{
  buffer__s *buf  = luaL_checkudata(L,1,TYPE_BUFFER);
  ptrdiff_t  posi = posrelat(luaL_optinteger(L,2,1),buf->idx);
  ptrdiff_t  pose = posrelat(luaL_optinteger(L,3,posi),buf->idx);
  int        n;
  int        i;
  
  if (posi <= 0) posi = 1;
  if ((size_t)pose > buf->idx) pose = buf->idx;
  if (posi > pose) return 0;
  n = (int)(pose - posi + 1);
  if (posi + n <= pose)
    luaL_error(L,"string slice too long");
  luaL_checkstack(L,n,"string slice too long");
  for (i = 0 ; i < n ; i++)
    lua_pushinteger(L,(unsigned char)buf->buf[posi+i-1]);
  return n;
}

static int bufferlua_new(lua_State *L)
{
  size_t len     = luaL_optinteger(L,1,1500);
  buffer__s *buf = lua_newuserdata(L,sizeof(buffer__s) + len);
  
  buf->len = len;
  buf->idx = 0;
  buf->buf = malloc(len);
  if (buf->buf == NULL)
  {
    lua_pushnil(L);
    return 1;
  }
  
  luaL_getmetatable(L,TYPE_BUFFER);
  lua_setmetatable(L,-2);
  return 1;
}
  
static const struct luaL_Reg m_buffer_meta[] =
{
  { "__gc"	, buffermeta___gc	} ,
  { "__len"	, buffermeta___len	} ,
  { "__tostring", buffermeta___tostring	} ,
  { "byte"	, buffermeta_byte	} , 
  { NULL	, NULL			}
};

static const struct luaL_Reg m_buffer_reg[] =
{
  { "new"	, bufferlua_new		} ,
  { NULL	, NULL			}
};

int luaopen_buffer(lua_State *L)
{
  luaL_newmetatable(L,TYPE_BUFFER);
  luaL_register(L,NULL,m_buffer_meta);
  lua_pushvalue(L,-1);
  lua_setfield(L,-2,"__index");
  
  luaL_register(L,"buffer",m_buffer_reg);
  return 1;
}
