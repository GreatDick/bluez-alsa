/*
 * test-at.c
 * Copyright (c) 2016-2021 Arkadiusz Bokowy
 *
 * This file is a part of bluez-alsa.
 *
 * This project is licensed under the terms of the MIT license.
 *
 */

#include <stdbool.h>
#include <string.h>

#include <check.h>

#include "at.h"
#include "hfp.h"

START_TEST(test_at_build) {

	char buffer[256];

	/* build commands */
	ck_assert_str_eq(at_build(buffer, sizeof(buffer), AT_TYPE_RAW, "\r\nRING", NULL), "\r\nRING");
	ck_assert_str_eq(at_build(buffer, sizeof(buffer), AT_TYPE_CMD, "+CLCC", NULL), "AT+CLCC\r");
	ck_assert_str_eq(at_build(buffer, sizeof(buffer), AT_TYPE_CMD_GET, "+COPS", NULL), "AT+COPS?\r");
	ck_assert_str_eq(at_build(buffer, sizeof(buffer), AT_TYPE_CMD_SET, "+BCS", "1"), "AT+BCS=1\r");
	ck_assert_str_eq(at_build(buffer, sizeof(buffer), AT_TYPE_CMD_TEST, "+CIND", NULL), "AT+CIND=?\r");

	/* build response result code */
	ck_assert_str_eq(at_build(buffer, sizeof(buffer), AT_TYPE_RESP, "+CIND", ""), "\r\n+CIND:\r\n");

	/* build unsolicited result code */
	ck_assert_str_eq(at_build(buffer, sizeof(buffer), AT_TYPE_RESP, NULL, "OK"), "\r\nOK\r\n");

} END_TEST

START_TEST(test_at_parse_invalid) {
	struct bt_at at;
	/* invalid AT command lines */
	ck_assert_ptr_eq(at_parse("ABC\r", &at), NULL);
	ck_assert_ptr_eq(at_parse("AT+CLCK?", &at), NULL);
	ck_assert_ptr_eq(at_parse("\r\r", &at), NULL);
	ck_assert_ptr_eq(at_parse("\r\nOK", &at), NULL);
} END_TEST

START_TEST(test_at_parse_cmd) {
	struct bt_at at;
	/* parse AT plain command */
	ck_assert_ptr_ne(at_parse("AT+CLCC\r", &at), NULL);
	ck_assert_int_eq(at.type, AT_TYPE_CMD);
	ck_assert_str_eq(at.command, "+CLCC");
	ck_assert_ptr_eq(at.value, NULL);
} END_TEST

START_TEST(test_at_parse_cmd_get) {
	struct bt_at at;
	/* parse AT GET command */
	ck_assert_ptr_ne(at_parse("AT+COPS?\r", &at), NULL);
	ck_assert_int_eq(at.type, AT_TYPE_CMD_GET);
	ck_assert_str_eq(at.command, "+COPS");
	ck_assert_ptr_eq(at.value, NULL);
} END_TEST

START_TEST(test_at_parse_cmd_set) {
	struct bt_at at;
	/* parse AT SET command */
	ck_assert_ptr_ne(at_parse("AT+CLCK=\"SC\",0,\"1234\"\r", &at), NULL);
	ck_assert_int_eq(at.type, AT_TYPE_CMD_SET);
	ck_assert_str_eq(at.command, "+CLCK");
	ck_assert_str_eq(at.value, "\"SC\",0,\"1234\"");
} END_TEST

START_TEST(test_at_parse_cmd_test) {
	struct bt_at at;
	/* parse AT TEST command */
	ck_assert_ptr_ne(at_parse("AT+COPS=?\r", &at), NULL);
	ck_assert_int_eq(at.type, AT_TYPE_CMD_TEST);
	ck_assert_str_eq(at.command, "+COPS");
	ck_assert_ptr_eq(at.value, NULL);
} END_TEST

START_TEST(test_at_parse_resp) {
	struct bt_at at;
	/* parse response result code */
	ck_assert_ptr_ne(at_parse("\r\n+CIND:0,0,1,4,0,4,0\r\n", &at), NULL);
	ck_assert_int_eq(at.type, AT_TYPE_RESP);
	ck_assert_str_eq(at.command, "+CIND");
	ck_assert_str_eq(at.value, "0,0,1,4,0,4,0");
} END_TEST

START_TEST(test_at_parse_resp_empty) {
	struct bt_at at;
	/* parse response result code with empty value */
	ck_assert_ptr_ne(at_parse("\r\n+CIND:\r\n", &at), NULL);
	ck_assert_int_eq(at.type, AT_TYPE_RESP);
	ck_assert_str_eq(at.command, "+CIND");
	ck_assert_str_eq(at.value, "");
} END_TEST

START_TEST(test_at_parse_resp_unsolicited) {
	struct bt_at at;
	/* parse unsolicited result code */
	ck_assert_ptr_ne(at_parse("\r\nRING\r\n", &at), NULL);
	ck_assert_int_eq(at.type, AT_TYPE_RESP);
	ck_assert_str_eq(at.command, "");
	ck_assert_str_eq(at.value, "RING");
} END_TEST

START_TEST(test_at_parse_case_sensitivity) {
	struct bt_at at;
	/* case-insensitive command and case-sensitive value */
	ck_assert_ptr_ne(at_parse("aT+tEsT=VaLuE\r", &at), NULL);
	ck_assert_int_eq(at.type, AT_TYPE_CMD_SET);
	ck_assert_str_eq(at.command, "+TEST");
	ck_assert_str_eq(at.value, "VaLuE");
} END_TEST

START_TEST(test_at_parse_multiple_cmds) {
	struct bt_at at;
	/* concatenated commands */
	const char *cmd = "\r\nOK\r\n\r\n+COPS:1\r\n";
	ck_assert_str_eq(at_parse(cmd, &at), &cmd[6]);
	ck_assert_int_eq(at.type, AT_TYPE_RESP);
	ck_assert_str_eq(at.command, "");
	ck_assert_str_eq(at.value, "OK");
} END_TEST

START_TEST(test_at_parse_bia) {

	const bool state_ok1[__HFP_IND_MAX] = { 0, true, true, true, true, true, true, true };
	const bool state_ok2[__HFP_IND_MAX] = { 0, true, false, true, true, true, true, false };
	const bool state_ok3[__HFP_IND_MAX] = { 0, true, false, false, false, true, true, true };
	const bool state_ok4[__HFP_IND_MAX] = { 0, true, true, false, false, true, true, true };
	bool state[__HFP_IND_MAX] = { 0 };

	ck_assert_int_eq(at_parse_bia("1,1,1,1,1,1,1", state), 0);
	ck_assert_int_eq(memcmp(state, state_ok1, sizeof(state)), 0);

	ck_assert_int_eq(at_parse_bia("1,0,1,1,1,1,0", state), 0);
	ck_assert_int_eq(memcmp(state, state_ok2, sizeof(state)), 0);

	/* omitted values shall not be changed */
	ck_assert_int_eq(at_parse_bia(",,0,0,,,1", state), 0);
	ck_assert_int_eq(memcmp(state, state_ok3, sizeof(state)), 0);

	/* truncated values shall not be changed */
	ck_assert_int_eq(at_parse_bia("1,1", state), 0);
	ck_assert_int_eq(memcmp(state, state_ok4, sizeof(state)), 0);

} END_TEST

START_TEST(test_at_parse_cind) {

	enum hfp_ind indmap[20];
	enum hfp_ind indmap_ok[20];

	/* parse +CIND response result code */
	ck_assert_int_eq(at_parse_cind("(\"call\",(0,1)),(\"xxx\",(0-3)),(\"signal\",(0-5))", indmap), 0);
	memset(indmap_ok, HFP_IND_NULL, sizeof(indmap_ok));
	indmap_ok[0] = HFP_IND_CALL;
	indmap_ok[2] = HFP_IND_SIGNAL;
	ck_assert_int_eq(memcmp(indmap, indmap_ok, sizeof(indmap)), 0);

	/* parse +CIND response with white-spaces */
	ck_assert_int_eq(at_parse_cind(" ( \"call\", ( 0, 1 ) ), ( \"signal\", ( 0-3 ) )", indmap), 0);

	/* parse +CIND invalid response */
	ck_assert_int_eq(at_parse_cind("(incorrect,1-2)", indmap), -1);

} END_TEST

START_TEST(test_at_parse_cmer) {

	const unsigned int cmer_ok1[5] = { 3, 0, 0, 1, 0 };
	const unsigned int cmer_ok2[5] = { 2, 0, 0, 1, 0 };
	unsigned int cmer[5];

	/* parse +CMER value */
	ck_assert_int_eq(at_parse_cmer("3,0,0,1,0", cmer), 0);
	ck_assert_int_eq(memcmp(cmer, cmer_ok1, sizeof(cmer)), 0);

	/* parse +CMER value with white-spaces */
	ck_assert_int_eq(at_parse_cmer("3, 0, 0 , 1 , 0", cmer), 0);
	ck_assert_int_eq(memcmp(cmer, cmer_ok1, sizeof(cmer)), 0);

	/* parse +CMER value with less elements */
	ck_assert_int_eq(at_parse_cmer("2,0", cmer), 0);
	ck_assert_int_eq(memcmp(cmer, cmer_ok2, sizeof(cmer)), 0);

	/* parse +CMER empty value */
	ck_assert_int_eq(at_parse_cmer("", cmer), 0);
	ck_assert_int_eq(memcmp(cmer, cmer_ok2, sizeof(cmer)), 0);

	/* parse +CMER invalid value */
	ck_assert_int_eq(at_parse_cmer("3,error", cmer), -1);

} END_TEST

START_TEST(test_at_type2str) {
	ck_assert_str_eq(at_type2str(AT_TYPE_RAW), "RAW");
	ck_assert_str_eq(at_type2str(AT_TYPE_RESP), "RESP");
} END_TEST

int main(void) {

	Suite *s = suite_create(__FILE__);
	TCase *tc = tcase_create(__FILE__);
	SRunner *sr = srunner_create(s);

	suite_add_tcase(s, tc);

	tcase_add_test(tc, test_at_build);
	tcase_add_test(tc, test_at_parse_invalid);
	tcase_add_test(tc, test_at_parse_cmd);
	tcase_add_test(tc, test_at_parse_cmd_get);
	tcase_add_test(tc, test_at_parse_cmd_set);
	tcase_add_test(tc, test_at_parse_cmd_test);
	tcase_add_test(tc, test_at_parse_resp);
	tcase_add_test(tc, test_at_parse_resp_empty);
	tcase_add_test(tc, test_at_parse_resp_unsolicited);
	tcase_add_test(tc, test_at_parse_case_sensitivity);
	tcase_add_test(tc, test_at_parse_multiple_cmds);
	tcase_add_test(tc, test_at_parse_bia);
	tcase_add_test(tc, test_at_parse_cind);
	tcase_add_test(tc, test_at_parse_cmer);
	tcase_add_test(tc, test_at_type2str);

	srunner_run_all(sr, CK_ENV);
	int nf = srunner_ntests_failed(sr);
	srunner_free(sr);

	return nf == 0 ? 0 : 1;
}
