#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include <errno.h>
#include <math.h>

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')

typedef struct {
    const char* json;
}lept_context;

static int lept_parse_literal(lept_context* c, lept_value* v,const char* ch, lept_type type)
{
    EXPECT(c, ch[0]);
    for (size_t i = 0; ch[i + 1]; i++,c->json++)
    {
        if (*(c->json) != ch[i+1])
            return LEPT_PARSE_INVALID_VALUE;
    }
    v->type = type;
    return LEPT_PARSE_OK;
}

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

#if  0
static int lept_parse_true(lept_context* c, lept_value* v) {
    EXPECT(c, 't');
    if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_TRUE;
    return LEPT_PARSE_OK;
}

static int lept_parse_false(lept_context* c, lept_value* v) {
    EXPECT(c, 'f');
    if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 4;
    v->type = LEPT_FALSE;
    return LEPT_PARSE_OK;
}

static int lept_parse_null(lept_context* c, lept_value* v) {
    EXPECT(c, 'n');
    if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_NULL;
    return LEPT_PARSE_OK;
}
#endif //  0




static int lept_parse_number(lept_context* c, lept_value* v) {
    /* \TODO validate number */
    /*
    number = [ "-" ] int [ frac ] [ exp ]
    int = "0" / digit1-9 *digit
    frac = "." 1*digit
    exp = ("e" / "E") ["-" / "+"] 1*digit
    */
    // [-]
    const char* p = c->json;
    if (*p == '-') p++;
    // int 首个是0，整数部分只有1个0
    if (*p == '0') p++;
    else {
        if (!ISDIGIT1TO9(*p)) return LEPT_PARSE_INVALID_VALUE;
        while (ISDIGIT(*p)) p++;
    }
    // .
    if (*p == '.')
    {
        p++;
        // frac
        if (!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;
        while (ISDIGIT(*p)) p++;
    }
    // exp
    if (*p == 'e' || *p == 'E')
    {
        p++;
        if (*p == '+' || *p == '-') p++;
        if (!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;
        while (ISDIGIT(*p)) p++;
    }

    // 不能修改c->json
    errno = 0;
    v->n = strtod(c->json, NULL);
    if (errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL)) return LEPT_PARSE_NUMBER_TOO_BIG;
    c->json = p;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
#if 0
    case 't':  return lept_parse_true(c, v);
    case 'f':  return lept_parse_false(c, v);
    case 'n':  return lept_parse_null(c, v);
#endif // 0
#if 1
    case 't':  return lept_parse_literal(c, v , "true" , LEPT_TRUE);
    case 'f':  return lept_parse_literal(c, v , "false", LEPT_FALSE);
    case 'n':  return lept_parse_literal(c, v , "null", LEPT_NULL);
#endif // 1
        default:   return lept_parse_number(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
