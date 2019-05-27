#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#include <libmnl/libmnl.h>
#include <linux/genetlink.h>
#include "phy_netlink.h"

static int phynl_fam;
static int mcgroup_id;

struct attr_tb_info {
	const struct nlattr **tb;
	unsigned int max_type;
};

static int cmd_cabdiag_req_cb(const struct nlmsghdr *nlhdr, void *data)
{
	struct nlattr *nestattr;
	struct nlattr *attr;
	int nesttype;
	int type;

	mnl_attr_for_each(attr, nlhdr, GENL_HDRLEN) {
		type = mnl_attr_get_type(attr);
		if (type == CABDIAG_OP_ATTR_REQUEST) {
			mnl_attr_for_each_nested(nestattr, attr) {
				nesttype = mnl_attr_get_type(nestattr);
				switch (nesttype) {
				case CABDIAG_REQ_ATTR_CMD:
					if (mnl_attr_get_u8(nestattr) ==
					    CMD_CABDIAG_START)
						printf("CMD : Start\n");
					else if (mnl_attr_get_u8(nestattr) ==
						 CMD_CABDIAG_STOP)
						printf("CMD : Stop\n");
					else
						printf("CMD : Invalid\n");
					break;
				case CABDIAG_REQ_ATTR_PAIRS_MASK:
					printf("Cable Pairs mask : 0x%x\n",
						mnl_attr_get_u8(nestattr));
					break;
				case CABDIAG_REQ_ATTR_TIMEOUT:
					printf("Timeout : %d\n",
						mnl_attr_get_u8(nestattr));
					break;
				}
			}
		}
	}

	return MNL_CB_OK;
}

static char *cabdiag_sta(u8 status)
{
	static char status_str[50];

	if (status == CD_NOT_SUPPORT) {
		sprintf(status_str, "Not Support");
		return status_str;
	}

	switch (status & 0xf) {
	case CD_NORMAL_PAIR:
		sprintf(status_str, "Correctly terminated pair");
		break;
	case CD_OPEN_PAIR:
		sprintf(status_str, "Open pair");
		break;
	case CD_SHORTED_PAIR:
		sprintf(status_str, "Shorted pair");
		break;
	case CD_ABNORMAL_TERMINATION:
		sprintf(status_str, "Abnormal termination");
		break;
	case CD_X_PAIR_SHORT_TO_PAIR_A:
		sprintf(status_str, "Cross-pair short to pair A");
		break;
	case CD_X_PAIR_SHORT_TO_PAIR_B:
		sprintf(status_str, "Cross-pair short to pair B");
		break;
	case CD_X_PAIR_SHORT_TO_PAIR_C:
		sprintf(status_str, "Cross-pair short to pair C");
		break;
	case CD_X_PAIR_SHORT_TO_PAIR_D:
		sprintf(status_str, "Cross-pair short to pair D");
		break;
	case CD_ABNORMAL_X_PAIR_A:
		sprintf(status_str, "Abnormal corss-piar coupling with pair A");
		break;
	case CD_ABNORMAL_X_PAIR_B:
		sprintf(status_str, "Abnormal corss-piar coupling with pair B");
		break;
	case CD_ABNORMAL_X_PAIR_C:
		sprintf(status_str, "Abnormal corss-piar coupling with pair C");
		break;
	case CD_ABNORMAL_X_PAIR_D:
		sprintf(status_str, "Abnormal corss-piar coupling with pair D");
		break;
	default:
		sprintf(status_str, "Un-known");
	}

	return status_str;
}

static void display_cabdiag_status(const struct phy_cabdiag_sta *status)
{
	printf("Cable diagnostics results:\n");
	if (status->pairs_bitmask & CABDIAG_PAIR_A_MASK)
		if (status->pairs[CABDIAG_PAIR_A].length != CD_NOT_SUPPORT)
			printf("    Pair A: %s, %dm\n",
			cabdiag_sta(status->pairs[CABDIAG_PAIR_A].status),
			status->pairs[CABDIAG_PAIR_A].length);
		else
			printf("    Pair A: %s, Length Not support\n",
			cabdiag_sta(status->pairs[CABDIAG_PAIR_A].status));
	if (status->pairs_bitmask & CABDIAG_PAIR_B_MASK)
		if (status->pairs[CABDIAG_PAIR_B].length != CD_NOT_SUPPORT)
			printf("    Pair B: %s, %dm\n",
			cabdiag_sta(status->pairs[CABDIAG_PAIR_B].status),
			status->pairs[CABDIAG_PAIR_B].length);
		else
			printf("    Pair B: %s, Length Not support\n",
			cabdiag_sta(status->pairs[CABDIAG_PAIR_B].status));
	if (status->pairs_bitmask & CABDIAG_PAIR_C_MASK)
		if (status->pairs[CABDIAG_PAIR_C].length != CD_NOT_SUPPORT)
			printf("    Pair C: %s, %dm\n",
			cabdiag_sta(status->pairs[CABDIAG_PAIR_C].status),
			status->pairs[CABDIAG_PAIR_C].length);
		else
			printf("    Pair C: %s, Length Not support\n",
			cabdiag_sta(status->pairs[CABDIAG_PAIR_C].status));
	if (status->pairs_bitmask & CABDIAG_PAIR_D_MASK)
		if (status->pairs[CABDIAG_PAIR_D].length != CD_NOT_SUPPORT)
			printf("    Pair D:  %s, %dm\n",
			cabdiag_sta(status->pairs[CABDIAG_PAIR_D].status),
			status->pairs[CABDIAG_PAIR_D].length);
		else
			printf("    Pair D:  %s, Length Not support\n",
			cabdiag_sta(status->pairs[CABDIAG_PAIR_D].status));
}

static int cmd_cabdiag_status_cb(const struct nlmsghdr *nlhdr, void *data)
{
	struct phy_cabdiag_sta status;
	unsigned long long pairs;
	struct nlattr *nestattr;
	struct nlattr *attr;
	int nesttype;
	int type;

	mnl_attr_for_each(attr, nlhdr, GENL_HDRLEN) {
		type = mnl_attr_get_type(attr);
		if (type == CABDIAG_OP_ATTR_STATUS) {
			mnl_attr_for_each_nested(nestattr, attr) {
				nesttype = mnl_attr_get_type(nestattr);
				switch (nesttype) {
				case CABDIAG_STA_ATTR_IFNAME:
					printf("Ethernet Intface name: %s\n",
						mnl_attr_get_str(nestattr));
					break;
				case CABDIAG_STA_ATTR_PAIRS_MASK:
					status.pairs_bitmask =
						mnl_attr_get_u8(nestattr);
					printf("Pairs Mask : 0x%x\n",
						status.pairs_bitmask);
					break;
				case CABDIAG_STA_ATTR_STATUS:
					pairs = mnl_attr_get_u64(nestattr);
					status.pairs[CABDIAG_PAIR_A].status =
						(u8) pairs & 0xff;
					status.pairs[CABDIAG_PAIR_A].length =
						(u8)(pairs >> 8) & 0xff;
					status.pairs[CABDIAG_PAIR_B].status =
						(u8)(pairs >> 16) & 0xff;
					status.pairs[CABDIAG_PAIR_B].length =
						(u8)(pairs >> 24) & 0xff;
					status.pairs[CABDIAG_PAIR_C].status =
						(u8)(pairs >> 32) & 0xff;
					status.pairs[CABDIAG_PAIR_C].length =
						(u8)(pairs >> 40) & 0xff;
					status.pairs[CABDIAG_PAIR_D].status =
						(u8)(pairs >> 48) & 0xff;
					status.pairs[CABDIAG_PAIR_D].length =
						(u8)(pairs >> 56) & 0xff;
					display_cabdiag_status(&status);
					break;
				}
			}
		}
	}

	return MNL_CB_OK;
}

int attr_cb(const struct nlattr *attr, void *data)
{
	const struct attr_tb_info *tb_info = data;
	int type = mnl_attr_get_type(attr);

	if (type >= 0 && type <= tb_info->max_type)
		tb_info->tb[type] = attr;

	return MNL_CB_OK;
}

static void find_monitor_group(struct nlattr *nest)
{
	const struct nlattr *grp_tb[CTRL_ATTR_MCAST_GRP_MAX + 1] = {};
	struct attr_tb_info grp_tb_info = { (grp_tb),
					    (MNL_ARRAY_SIZE(grp_tb) - 1) };
	struct nlattr *grp_attr;
	int ret;

	mnl_attr_for_each_nested(grp_attr, nest) {
		ret = mnl_attr_parse_nested(grp_attr, attr_cb, &grp_tb_info);
		if (ret < 0)
			return;
		if (!grp_tb[CTRL_ATTR_MCAST_GRP_NAME] ||
			!grp_tb[CTRL_ATTR_MCAST_GRP_ID])
			continue;

		mcgroup_id = mnl_attr_get_u32(grp_tb[CTRL_ATTR_MCAST_GRP_ID]);
		printf("GroupID = %d\n", mcgroup_id);
		printf("GroupName = %s\n",
			mnl_attr_get_str(grp_tb[CTRL_ATTR_MCAST_GRP_NAME]));
		return;
	}
}

static int ethnl_family_cb(const struct nlmsghdr *nlhdr, void *data)
{
	struct nlattr *attr;
	int type;

	phynl_fam = 0;
	mnl_attr_for_each(attr, nlhdr, GENL_HDRLEN) {
		type = mnl_attr_get_type(attr);
		if (type == CTRL_ATTR_FAMILY_ID)
			phynl_fam = mnl_attr_get_u16(attr);
		if (type == CTRL_ATTR_MCAST_GROUPS)
			find_monitor_group(attr);
	}

	return (phynl_fam ? MNL_CB_OK : MNL_CB_ERROR);
}

int main(int argc, char *argv[])
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	int ret, len, numbytes, hdrsize;
	struct genlmsghdr *genl;
	struct mnl_socket *nl;
	struct nlmsghdr *nlh;
	struct nlattr *nest;
	unsigned int seq;
	u8 cmd;

	seq = time(NULL);

	/*
	 * step1 : find "microchipphy" netlink family and get family id
	 *
	 * This is a necessary init procedure before other calls to
	 * our own netlink module
	 */
	memset(buf, 0, sizeof(buf));
	nlh = mnl_nlmsg_put_header(buf);
	nlh->nlmsg_type = GENL_ID_CTRL;
	nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
	nlh->nlmsg_seq = seq;

	hdrsize = sizeof(struct genlmsghdr);
	genl = mnl_nlmsg_put_extra_header(nlh, hdrsize);
	genl->cmd = CTRL_CMD_GETFAMILY;
	genl->version = 1;

	/* get genl family id */
	mnl_attr_put_strz(nlh, CTRL_ATTR_FAMILY_NAME, "phynl");

	nl = mnl_socket_open(NETLINK_GENERIC);
	if (nl == NULL) {
		perror("mnl_socket_open");
		exit(EXIT_FAILURE);
	}

	if (mnl_socket_bind(nl, 0, 0) < 0) {
		perror("mnl_socket_bind");
		exit(EXIT_FAILURE);
	}

	if (mnl_socket_sendto(nl, nlh, nlh->nlmsg_len) < 0) {
		perror("mnl_socket_sendto");
		exit(EXIT_FAILURE);
	}

	/* ethnl_family_cb is called and get phynl_fam */
	while ((len = mnl_socket_recvfrom(nl, buf, sizeof(buf))) > 0) {
		ret = mnl_cb_run(buf, len, seq, 0, ethnl_family_cb, NULL);
		if (ret <= 0)
			break;
	}

	if (ret == -1) {
		perror("error 1");
		exit(EXIT_FAILURE);
	}

	if (argc == 4 && !strcmp(argv[2], "request")) {
		cmd = atoi(argv[3]);
		/* Send messages to our netlink module cable diag request */
		printf("\nCable Diagnostics Request:\n");
		memset(buf, 0, sizeof(buf));
		nlh = mnl_nlmsg_put_header(buf);
		nlh->nlmsg_type = phynl_fam;
		nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
		nlh->nlmsg_seq = ++seq;

		hdrsize = sizeof(struct genlmsghdr);
		genl = mnl_nlmsg_put_extra_header(nlh, hdrsize);
		genl->cmd = PHYNL_CMD_CABDIAG;
		genl->version = 1;

		/* start nest attrs */
		nest = mnl_attr_nest_start(nlh, CABDIAG_OP_ATTR_REQUEST);
		if (!nest) {
			perror("mnl_attr_nest_start");
			exit(EXIT_FAILURE);
		}

		/* Pass device name to kernel */
		mnl_attr_put_strz(nlh, CABDIAG_REQ_ATTR_IFNAME, argv[1]);
		/* Cable diag cmd (start/stop) */
		if (cmd == CMD_CABDIAG_START)
			mnl_attr_put_u8(nlh, CABDIAG_REQ_ATTR_CMD,
					CMD_CABDIAG_START);
		else if (cmd == CMD_CABDIAG_STOP)
			mnl_attr_put_u8(nlh, CABDIAG_REQ_ATTR_CMD,
					CMD_CABDIAG_STOP);
		else
			mnl_attr_put_u8(nlh, CABDIAG_REQ_ATTR_CMD,
					CMD_CABDIAG_NONE);
		mnl_attr_put_u8(nlh, CABDIAG_REQ_ATTR_PAIRS_MASK,
				(u8)(CABDIAG_PAIR_A_MASK));
		mnl_attr_put_u8(nlh, CABDIAG_REQ_ATTR_TIMEOUT, 10);
		mnl_attr_nest_end(nlh, nest);

		if (mnl_socket_sendto(nl, nlh, nlh->nlmsg_len) < 0) {
			perror("mnl_socket_sendto");
			exit(EXIT_FAILURE);
		}

		while ((len = mnl_socket_recvfrom(nl, buf, sizeof(buf))) > 0) {
			ret = mnl_cb_run(buf, len, seq, 0,
					 cmd_cabdiag_req_cb, NULL);
			if (ret <= 0)
				break;
		}
	}

	else if (argc == 3 && !strcmp(argv[2], "status")) {
		/* Send messages to our netlink module cable diag status */
		printf("\nCable Diagnostics Status:\n");
		memset(buf, 0, sizeof(buf));
		nlh = mnl_nlmsg_put_header(buf);
		nlh->nlmsg_type = phynl_fam;
		nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
		nlh->nlmsg_seq = ++seq;

		hdrsize = sizeof(struct genlmsghdr);
		genl = mnl_nlmsg_put_extra_header(nlh, hdrsize);
		genl->cmd = PHYNL_CMD_CABDIAG;
		genl->version = 1;

		/* Start nest attrs */
		nest = mnl_attr_nest_start(nlh, CABDIAG_OP_ATTR_STATUS);
		if (!nest) {
			perror("mnl_attr_nest_start");
			exit(EXIT_FAILURE);
		}

		mnl_attr_put_strz(nlh, CABDIAG_STA_ATTR_IFNAME, argv[1]);
		mnl_attr_put_u8(nlh, CABDIAG_STA_ATTR_PAIRS_MASK,
				(CABDIAG_PAIR_A_MASK));
		mnl_attr_nest_end(nlh, nest);

		if (mnl_socket_sendto(nl, nlh, nlh->nlmsg_len) < 0) {
			perror("mnl_socket_sendto");
			exit(EXIT_FAILURE);
		}

		while ((len = mnl_socket_recvfrom(nl, buf, sizeof(buf))) > 0) {
			ret = mnl_cb_run(buf, len, seq, 0,
					 cmd_cabdiag_status_cb, NULL);
			if (ret <= 0)
				break;
		}
	}

	if (ret == -1) {
		perror("error 2");
		exit(EXIT_FAILURE);
	}

	mnl_socket_close(nl);

	return 0;
}

