/* parse/f-info */
/* Exercise parsing used for terrain.txt. */

#include "unit-test.h"
#include "unit-test-data.h"
#include "cave.h"
#include "init.h"
#include "monster.h"
#include <locale.h>
#include <langinfo.h>


int setup_tests(void **state) {
	*state = init_parse_feat();
	return !*state;
}

int teardown_tests(void *state) {
	struct parser *p = (struct parser*) state;
	struct feature *f = (struct feature*) parser_priv(p);

	string_free(f->look_in_preposition);
	string_free(f->look_prefix);
	string_free(f->confused_msg);
	string_free(f->die_msg);
	string_free(f->hurt_msg);
	string_free(f->run_msg);
	string_free(f->walk_msg);
	string_free(f->mimic);
	string_free(f->desc);
	string_free(f->name);
	mem_free(f);
	parser_destroy(p);
	return 0;
}

static int test_missing_header_record0(void *state) {
	struct parser *p = (struct parser*) state;
	struct feature *f = (struct feature*) parser_priv(p);
	enum parser_error r;

	null(f);
	r = parser_parse(p, "graphics: :w");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "priority:2");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "mimic:granite wall");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "flags:LOS | PASSABLE");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "info:8:0");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	ok;
}

static int test_name0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "name:Test Feature");
	struct feature *f;

	eq(r, PARSE_ERROR_NONE);
	f = (struct feature*) parser_priv(p);
	notnull(f);
	require(streq(f->name, "Test Feature"));
	null(f->desc);
	null(f->mimic);
	eq(f->priority, 0);
	eq(f->shopnum, 0);
	eq(f->dig, 0);
	require(flag_is_empty(f->flags, TF_SIZE));
	eq(f->d_attr, 0);
	eq(f->d_char, 0);
	null(f->walk_msg);
	null(f->run_msg);
	null(f->hurt_msg);
	null(f->die_msg);
	null(f->confused_msg);
	null(f->look_prefix);
	null(f->look_in_preposition);
	eq(f->resist_flag, 0);
	ok;
}

static int test_graphics0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "graphics:::Light Green");
	struct feature *f;

	eq(r, PARSE_ERROR_NONE);
	f = (struct feature*) parser_priv(p);
	notnull(f);
	eq(f->d_char, L':');
	eq(f->d_attr, COLOUR_L_GREEN);
	/* Check that single letter code for color works. */
	r = parser_parse(p, "graphics:^:b");
	eq(r, PARSE_ERROR_NONE);
	f = (struct feature*) parser_priv(p);
	notnull(f);
	eq(f->d_char, L'^');
	eq(f->d_attr, COLOUR_BLUE);
	/* Check that full name matching for color is case insensitive. */
	r = parser_parse(p, "graphics:#:light purple");
	f = (struct feature*) parser_priv(p);
	notnull(f);
	eq(f->d_char, L'#');
	eq(f->d_attr, COLOUR_L_PURPLE);
	if (setlocale(LC_CTYPE, "") && streq(nl_langinfo(CODESET), "UTF-8")) {
		/*
		 * Check for glyph that is outside of the ASCII range.  Use
		 * the Yen sign, Unicode U+00A5 or C2 A5 as UTF-8.
		 */
		wchar_t wcs[3];
		size_t nc;

		r = parser_parse(p, "graphics:¥:red");
		eq(r, PARSE_ERROR_NONE);
		nc = text_mbstowcs(wcs, "¥", (int) N_ELEMENTS(wcs));
		eq(nc, 1);
		f = (struct feature*) parser_priv(p);
		notnull(f);
		eq(f->d_char, wcs[0]);
		eq(f->d_attr, COLOUR_RED);
	}
	ok;
}

static int test_mimic0(void *state) {
	enum parser_error r = parser_parse(state, "mimic:marshmallow");
	struct feature *f;

	eq(r, PARSE_ERROR_NONE);
	f = parser_priv(state);
	require(f);
	require(streq(f->mimic, "marshmallow"));
	ok;
}

static int test_priority0(void *state) {
	enum parser_error r = parser_parse(state, "priority:2");
	struct feature *f;

	eq(r, PARSE_ERROR_NONE);
	f = parser_priv(state);
	require(f);
	eq(f->priority, 2);
	ok;
}

static int test_flags0(void *state) {
	struct parser *p = (struct parser*) state;
	struct feature *f = (struct feature*) parser_priv(p);
	enum parser_error r;
	bitflag eflags[TF_SIZE];

	notnull(f);
	flag_wipe(f->flags, TF_SIZE);
	/* Check that specifying no flags works. */
	r = parser_parse(p, "flags:");
	eq(r, PARSE_ERROR_NONE);
	f = (struct feature*) parser_priv(p);
	notnull(f);
	require(flag_is_empty(f->flags, TF_SIZE));
	/* Try setting one flag. */
	r = parser_parse(p, "flags:LOS");
	eq(r, PARSE_ERROR_NONE);
	/* Try setting more than one flag. */
	r = parser_parse(state, "flags:PERMANENT | DOWNSTAIR");
	eq(r, PARSE_ERROR_NONE);
	f = (struct feature*) parser_priv(p);
	notnull(f);
	flag_wipe(eflags, TF_SIZE);
	flag_on_dbg(eflags, TF_SIZE, TF_LOS, "eflags", "TF_LOS");
	flag_on_dbg(eflags, TF_SIZE, TF_PERMANENT, "eflags", "TF_PERMANENT");
	flag_on_dbg(eflags, TF_SIZE, TF_DOWNSTAIR, "eflags", "TF_DOWNSTAIR");
	require(flag_is_equal(f->flags, eflags, TF_SIZE));
	ok;
}

static int test_info0(void *state) {
	enum parser_error r = parser_parse(state, "info:9:2");
	struct feature *f;

	eq(r, PARSE_ERROR_NONE);
	f = parser_priv(state);
	require(f);
	eq(f->shopnum, 9);
	eq(f->dig, 2);
	ok;
}

static int test_desc0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p,
		"desc:A door that is already open.");
	struct feature *f;

	eq(r, PARSE_ERROR_NONE);
	/* Check that additional directives are appended to the first. */
	r = parser_parse(p, "desc:  Player, monster, spell, and missile "
		"can pass through as long as it stays open.");
	eq(r, PARSE_ERROR_NONE);
	f = (struct feature*) parser_priv(p);
	notnull(f);
	notnull(f->desc);
	require(streq(f->desc, "A door that is already open.  Player, "
		"monster, spell, and missile can pass through as long as it "
		"stays open."));
	ok;
}

static int test_walk_msg0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "walk-msg:It looks dangerous.");
	struct feature *f;

	eq(r, PARSE_ERROR_NONE);
	/* Check that additional directives are appended to the first. */
	r = parser_parse(p, "walk-msg:  Really enter? ");
	eq(r, PARSE_ERROR_NONE);
	f = (struct feature*) parser_priv(p);
	notnull(f);
	notnull(f->walk_msg);
	require(streq(f->walk_msg, "It looks dangerous.  Really enter? "));
	ok;
}

static int test_run_msg0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "run-msg:It blocks your path.");
	struct feature *f;

	eq(r, PARSE_ERROR_NONE);
	/* Check that additional directives are appended to the first. */
	r = parser_parse(p, "run-msg:  Really enter? ");
	eq(r, PARSE_ERROR_NONE);
	f = (struct feature*) parser_priv(p);
	notnull(f);
	notnull(f->run_msg);
	require(streq(f->run_msg, "It blocks your path.  Really enter? "));
	ok;
}

static int test_hurt_msg0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "hurt-msg:Ow!");
	struct feature *f;

	eq(r, PARSE_ERROR_NONE);
	/* Check that additional directives are appended to the first. */
	r = parser_parse(p, "hurt-msg:  That hurt!");
	eq(r, PARSE_ERROR_NONE);
	f = (struct feature*) parser_priv(p);
	notnull(f);
	notnull(f->hurt_msg);
	require(streq(f->hurt_msg, "Ow!  That hurt!"));
	ok;
}

static int test_die_msg0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "die-msg:dissolving");
	struct feature *f;

	eq(r, PARSE_ERROR_NONE);
	/* Check that additional directives are appended to the first. */
	r = parser_parse(p, "die-msg: in a pool of acid");
	eq(r, PARSE_ERROR_NONE);
	f = (struct feature*) parser_priv(p);
	notnull(f);
	notnull(f->die_msg);
	require(streq(f->die_msg, "dissolving in a pool of acid"));
	ok;
}

static int test_confused_msg0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "confused-msg:slams into a wall");
	struct feature *f;

	eq(r, PARSE_ERROR_NONE);
	/* Check that additional directives are appended to the first. */
	r = parser_parse(p, "confused-msg: and stumbles");
	eq(r, PARSE_ERROR_NONE);
	f = (struct feature*) parser_priv(p);
	notnull(f);
	notnull(f->confused_msg);
	require(streq(f->confused_msg, "slams into a wall and stumbles"));
	ok;
}

static int test_look_prefix0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "look-prefix:the entrance ");
	struct feature *f;

	eq(r, PARSE_ERROR_NONE);
	/* Check that additional directives are appended to the first. */
	r = parser_parse(p, "look-prefix:to the");
	eq(r, PARSE_ERROR_NONE);
	f = (struct feature*) parser_priv(p);
	notnull(f);
	notnull(f->look_prefix);
	require(streq(f->look_prefix, "the entrance to the"));
	ok;
}

static int test_look_in_preposition0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "look-in-preposition:at the ");
	struct feature *f;

	eq(r, PARSE_ERROR_NONE);
	/* Check that additional directives are appended to the first. */
	r = parser_parse(p, "look-in-preposition:brink of");
	eq(r, PARSE_ERROR_NONE);
	f = (struct feature*) parser_priv(p);
	notnull(f);
	notnull(f->look_in_preposition);
	require(streq(f->look_in_preposition, "at the brink of"));
	ok;
}

static int test_resist_flag0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "resist-flag:IM_POIS");
	struct feature *f;

	eq(r, PARSE_ERROR_NONE);
	f = (struct feature*) parser_priv(p);
	notnull(f);
	eq(f->resist_flag, RF_IM_POIS);
	ok;
}

static int test_resist_flag_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try an unrecognized race flag. */
	enum parser_error r = parser_parse(p, "resist-flag:XYZZY");

	eq(r, PARSE_ERROR_INVALID_FLAG);
	ok;
}

const char *suite_name = "parse/f-info";
/* test_missing_header_record0() has to be before test_name0(). */
struct test tests[] = {
	{ "missing_header_record0", test_missing_header_record0 },
	{ "name0", test_name0 },
	{ "graphics0", test_graphics0 },
	{ "mimic0", test_mimic0 },
	{ "priority0", test_priority0 },
	{ "flags0", test_flags0 },
	{ "info0", test_info0 },
	{ "desc0", test_desc0 },
	{ "walk_msg0", test_walk_msg0 },
	{ "run_msg0", test_run_msg0 },
	{ "hurt_msg0", test_hurt_msg0 },
	{ "die_msg0", test_die_msg0 },
	{ "confused_msg0", test_confused_msg0 },
	{ "look_prefix0", test_look_prefix0 },
	{ "look_in_preposition0", test_look_in_preposition0 },
	{ "resist_flag0", test_resist_flag0 },
	{ "resist_flag_bad0", test_resist_flag_bad0 },
	{ NULL, NULL }
};
