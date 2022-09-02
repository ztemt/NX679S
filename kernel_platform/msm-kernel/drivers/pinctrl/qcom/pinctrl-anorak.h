/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#define FUNCTION(fname)			                \
	[msm_mux_##fname] = {		                \
		.name = #fname,				\
		.groups = fname##_groups,               \
		.ngroups = ARRAY_SIZE(fname##_groups),	\
	}

#define REG_BASE 0x100000
#define REG_SIZE 0x1000
#define PINGROUP(id, f1, f2, f3, f4, f5, f6, f7, f8, f9, wake_off, bit)	\
	{					        \
		.name = "gpio" #id,			\
		.pins = gpio##id##_pins,		\
		.npins = (unsigned int)ARRAY_SIZE(gpio##id##_pins),	\
		.funcs = (int[]){			\
			msm_mux_gpio, /* gpio mode */	\
			msm_mux_##f1,			\
			msm_mux_##f2,			\
			msm_mux_##f3,			\
			msm_mux_##f4,			\
			msm_mux_##f5,			\
			msm_mux_##f6,			\
			msm_mux_##f7,			\
			msm_mux_##f8,			\
			msm_mux_##f9			\
		},				        \
		.nfuncs = 10,				\
		.ctl_reg = REG_BASE + REG_SIZE * id,			\
		.io_reg = REG_BASE + 0x4 + REG_SIZE * id,		\
		.intr_cfg_reg = REG_BASE + 0x8 + REG_SIZE * id,		\
		.intr_status_reg = REG_BASE + 0xc + REG_SIZE * id,	\
		.intr_target_reg = REG_BASE + 0x8 + REG_SIZE * id,	\
		.mux_bit = 2,			\
		.pull_bit = 0,			\
		.drv_bit = 6,			\
		.oe_bit = 9,			\
		.in_bit = 0,			\
		.out_bit = 1,			\
		.intr_enable_bit = 0,		\
		.intr_status_bit = 0,		\
		.intr_target_bit = 5,		\
		.intr_target_kpss_val = 3,	\
		.intr_raw_status_bit = 4,	\
		.intr_polarity_bit = 1,		\
		.intr_detection_bit = 2,	\
		.intr_detection_width = 2,	\
		.wake_reg = REG_BASE + wake_off,	\
		.wake_bit = bit,		\
	}

#define SDC_QDSD_PINGROUP(pg_name, ctl, pull, drv)	\
	{					        \
		.name = #pg_name,			\
		.pins = pg_name##_pins,			\
		.npins = (unsigned int)ARRAY_SIZE(pg_name##_pins),	\
		.ctl_reg = ctl,				\
		.io_reg = 0,				\
		.intr_cfg_reg = 0,			\
		.intr_status_reg = 0,			\
		.intr_target_reg = 0,			\
		.mux_bit = -1,				\
		.pull_bit = pull,			\
		.drv_bit = drv,				\
		.oe_bit = -1,				\
		.in_bit = -1,				\
		.out_bit = -1,				\
		.intr_enable_bit = -1,			\
		.intr_status_bit = -1,			\
		.intr_target_bit = -1,			\
		.intr_raw_status_bit = -1,		\
		.intr_polarity_bit = -1,		\
		.intr_detection_bit = -1,		\
		.intr_detection_width = -1,		\
	}

#define UFS_RESET(pg_name, offset)				\
	{					        \
		.name = #pg_name,			\
		.pins = pg_name##_pins,			\
		.npins = (unsigned int)ARRAY_SIZE(pg_name##_pins),	\
		.ctl_reg = offset,			\
		.io_reg = offset + 0x4,			\
		.intr_cfg_reg = 0,			\
		.intr_status_reg = 0,			\
		.intr_target_reg = 0,			\
		.mux_bit = -1,				\
		.pull_bit = 3,				\
		.drv_bit = 0,				\
		.oe_bit = -1,				\
		.in_bit = -1,				\
		.out_bit = 0,				\
		.intr_enable_bit = -1,			\
		.intr_status_bit = -1,			\
		.intr_target_bit = -1,			\
		.intr_raw_status_bit = -1,		\
		.intr_polarity_bit = -1,		\
		.intr_detection_bit = -1,		\
		.intr_detection_width = -1,		\
	}

#define QUP_I3C(qup_mode, qup_offset)			\
	{						\
		.mode = qup_mode,			\
		.offset = qup_offset,			\
	}


static const struct pinctrl_pin_desc anorak_pins[] = {
	PINCTRL_PIN(0, "GPIO_0"),
	PINCTRL_PIN(1, "GPIO_1"),
	PINCTRL_PIN(2, "GPIO_2"),
	PINCTRL_PIN(3, "GPIO_3"),
	PINCTRL_PIN(4, "GPIO_4"),
	PINCTRL_PIN(5, "GPIO_5"),
	PINCTRL_PIN(6, "GPIO_6"),
	PINCTRL_PIN(7, "GPIO_7"),
	PINCTRL_PIN(8, "GPIO_8"),
	PINCTRL_PIN(9, "GPIO_9"),
	PINCTRL_PIN(10, "GPIO_10"),
	PINCTRL_PIN(11, "GPIO_11"),
	PINCTRL_PIN(12, "GPIO_12"),
	PINCTRL_PIN(13, "GPIO_13"),
	PINCTRL_PIN(14, "GPIO_14"),
	PINCTRL_PIN(15, "GPIO_15"),
	PINCTRL_PIN(16, "GPIO_16"),
	PINCTRL_PIN(17, "GPIO_17"),
	PINCTRL_PIN(18, "GPIO_18"),
	PINCTRL_PIN(19, "GPIO_19"),
	PINCTRL_PIN(20, "GPIO_20"),
	PINCTRL_PIN(21, "GPIO_21"),
	PINCTRL_PIN(22, "GPIO_22"),
	PINCTRL_PIN(23, "GPIO_23"),
	PINCTRL_PIN(24, "GPIO_24"),
	PINCTRL_PIN(25, "GPIO_25"),
	PINCTRL_PIN(26, "GPIO_26"),
	PINCTRL_PIN(27, "GPIO_27"),
	PINCTRL_PIN(28, "GPIO_28"),
	PINCTRL_PIN(29, "GPIO_29"),
	PINCTRL_PIN(30, "GPIO_30"),
	PINCTRL_PIN(31, "GPIO_31"),
	PINCTRL_PIN(32, "GPIO_32"),
	PINCTRL_PIN(33, "GPIO_33"),
	PINCTRL_PIN(34, "GPIO_34"),
	PINCTRL_PIN(35, "GPIO_35"),
	PINCTRL_PIN(36, "GPIO_36"),
	PINCTRL_PIN(37, "GPIO_37"),
	PINCTRL_PIN(38, "GPIO_38"),
	PINCTRL_PIN(39, "GPIO_39"),
	PINCTRL_PIN(40, "GPIO_40"),
	PINCTRL_PIN(41, "GPIO_41"),
	PINCTRL_PIN(42, "GPIO_42"),
	PINCTRL_PIN(43, "GPIO_43"),
	PINCTRL_PIN(44, "GPIO_44"),
	PINCTRL_PIN(45, "GPIO_45"),
	PINCTRL_PIN(46, "GPIO_46"),
	PINCTRL_PIN(47, "GPIO_47"),
	PINCTRL_PIN(48, "GPIO_48"),
	PINCTRL_PIN(49, "GPIO_49"),
	PINCTRL_PIN(50, "GPIO_50"),
	PINCTRL_PIN(51, "GPIO_51"),
	PINCTRL_PIN(52, "GPIO_52"),
	PINCTRL_PIN(53, "GPIO_53"),
	PINCTRL_PIN(54, "GPIO_54"),
	PINCTRL_PIN(55, "GPIO_55"),
	PINCTRL_PIN(56, "GPIO_56"),
	PINCTRL_PIN(57, "GPIO_57"),
	PINCTRL_PIN(58, "GPIO_58"),
	PINCTRL_PIN(59, "GPIO_59"),
	PINCTRL_PIN(60, "GPIO_60"),
	PINCTRL_PIN(61, "GPIO_61"),
	PINCTRL_PIN(62, "GPIO_62"),
	PINCTRL_PIN(63, "GPIO_63"),
	PINCTRL_PIN(64, "GPIO_64"),
	PINCTRL_PIN(65, "GPIO_65"),
	PINCTRL_PIN(66, "GPIO_66"),
	PINCTRL_PIN(67, "GPIO_67"),
	PINCTRL_PIN(68, "GPIO_68"),
	PINCTRL_PIN(69, "GPIO_69"),
	PINCTRL_PIN(70, "GPIO_70"),
	PINCTRL_PIN(71, "GPIO_71"),
	PINCTRL_PIN(72, "GPIO_72"),
	PINCTRL_PIN(73, "GPIO_73"),
	PINCTRL_PIN(74, "GPIO_74"),
	PINCTRL_PIN(75, "GPIO_75"),
	PINCTRL_PIN(76, "GPIO_76"),
	PINCTRL_PIN(77, "GPIO_77"),
	PINCTRL_PIN(78, "GPIO_78"),
	PINCTRL_PIN(79, "GPIO_79"),
	PINCTRL_PIN(80, "GPIO_80"),
	PINCTRL_PIN(81, "GPIO_81"),
	PINCTRL_PIN(82, "GPIO_82"),
	PINCTRL_PIN(83, "GPIO_83"),
	PINCTRL_PIN(84, "GPIO_84"),
	PINCTRL_PIN(85, "GPIO_85"),
	PINCTRL_PIN(86, "GPIO_86"),
	PINCTRL_PIN(87, "GPIO_87"),
	PINCTRL_PIN(88, "GPIO_88"),
	PINCTRL_PIN(89, "GPIO_89"),
	PINCTRL_PIN(90, "GPIO_90"),
	PINCTRL_PIN(91, "GPIO_91"),
	PINCTRL_PIN(92, "GPIO_92"),
	PINCTRL_PIN(93, "GPIO_93"),
	PINCTRL_PIN(94, "GPIO_94"),
	PINCTRL_PIN(95, "GPIO_95"),
	PINCTRL_PIN(96, "GPIO_96"),
	PINCTRL_PIN(97, "GPIO_97"),
	PINCTRL_PIN(98, "GPIO_98"),
	PINCTRL_PIN(99, "GPIO_99"),
	PINCTRL_PIN(100, "GPIO_100"),
	PINCTRL_PIN(101, "GPIO_101"),
	PINCTRL_PIN(102, "GPIO_102"),
	PINCTRL_PIN(103, "GPIO_103"),
	PINCTRL_PIN(104, "GPIO_104"),
	PINCTRL_PIN(105, "GPIO_105"),
	PINCTRL_PIN(106, "GPIO_106"),
	PINCTRL_PIN(107, "GPIO_107"),
	PINCTRL_PIN(108, "GPIO_108"),
	PINCTRL_PIN(109, "GPIO_109"),
	PINCTRL_PIN(110, "GPIO_110"),
	PINCTRL_PIN(111, "GPIO_111"),
	PINCTRL_PIN(112, "GPIO_112"),
	PINCTRL_PIN(113, "GPIO_113"),
	PINCTRL_PIN(114, "GPIO_114"),
	PINCTRL_PIN(115, "GPIO_115"),
	PINCTRL_PIN(116, "GPIO_116"),
	PINCTRL_PIN(117, "GPIO_117"),
	PINCTRL_PIN(118, "GPIO_118"),
	PINCTRL_PIN(119, "GPIO_119"),
	PINCTRL_PIN(120, "GPIO_120"),
	PINCTRL_PIN(121, "GPIO_121"),
	PINCTRL_PIN(122, "GPIO_122"),
	PINCTRL_PIN(123, "GPIO_123"),
	PINCTRL_PIN(124, "GPIO_124"),
	PINCTRL_PIN(125, "GPIO_125"),
	PINCTRL_PIN(126, "GPIO_126"),
	PINCTRL_PIN(127, "GPIO_127"),
	PINCTRL_PIN(128, "GPIO_128"),
	PINCTRL_PIN(129, "GPIO_129"),
	PINCTRL_PIN(130, "GPIO_130"),
	PINCTRL_PIN(131, "GPIO_131"),
	PINCTRL_PIN(132, "GPIO_132"),
	PINCTRL_PIN(133, "GPIO_133"),
	PINCTRL_PIN(134, "GPIO_134"),
	PINCTRL_PIN(135, "GPIO_135"),
	PINCTRL_PIN(136, "GPIO_136"),
	PINCTRL_PIN(137, "GPIO_137"),
	PINCTRL_PIN(138, "GPIO_138"),
	PINCTRL_PIN(139, "GPIO_139"),
	PINCTRL_PIN(140, "GPIO_140"),
	PINCTRL_PIN(141, "GPIO_141"),
	PINCTRL_PIN(142, "GPIO_142"),
	PINCTRL_PIN(143, "GPIO_143"),
	PINCTRL_PIN(144, "GPIO_144"),
	PINCTRL_PIN(145, "GPIO_145"),
	PINCTRL_PIN(146, "GPIO_146"),
	PINCTRL_PIN(147, "GPIO_147"),
	PINCTRL_PIN(148, "GPIO_148"),
	PINCTRL_PIN(149, "GPIO_149"),
	PINCTRL_PIN(150, "GPIO_150"),
	PINCTRL_PIN(151, "GPIO_151"),
	PINCTRL_PIN(152, "GPIO_152"),
	PINCTRL_PIN(153, "GPIO_153"),
	PINCTRL_PIN(154, "GPIO_154"),
	PINCTRL_PIN(155, "GPIO_155"),
	PINCTRL_PIN(156, "GPIO_156"),
	PINCTRL_PIN(157, "GPIO_157"),
	PINCTRL_PIN(158, "GPIO_158"),
	PINCTRL_PIN(159, "GPIO_159"),
	PINCTRL_PIN(160, "GPIO_160"),
	PINCTRL_PIN(161, "GPIO_161"),
	PINCTRL_PIN(162, "GPIO_162"),
	PINCTRL_PIN(163, "GPIO_163"),
	PINCTRL_PIN(164, "GPIO_164"),
	PINCTRL_PIN(165, "GPIO_165"),
	PINCTRL_PIN(166, "GPIO_166"),
	PINCTRL_PIN(167, "GPIO_167"),
	PINCTRL_PIN(168, "GPIO_168"),
	PINCTRL_PIN(169, "GPIO_169"),
	PINCTRL_PIN(170, "GPIO_170"),
	PINCTRL_PIN(171, "GPIO_171"),
	PINCTRL_PIN(172, "GPIO_172"),
	PINCTRL_PIN(173, "GPIO_173"),
	PINCTRL_PIN(174, "GPIO_174"),
	PINCTRL_PIN(175, "GPIO_175"),
	PINCTRL_PIN(176, "GPIO_176"),
	PINCTRL_PIN(177, "GPIO_177"),
	PINCTRL_PIN(178, "GPIO_178"),
	PINCTRL_PIN(179, "GPIO_179"),
	PINCTRL_PIN(180, "GPIO_180"),
	PINCTRL_PIN(181, "GPIO_181"),
	PINCTRL_PIN(182, "GPIO_182"),
	PINCTRL_PIN(183, "GPIO_183"),
	PINCTRL_PIN(184, "GPIO_184"),
	PINCTRL_PIN(185, "GPIO_185"),
	PINCTRL_PIN(186, "GPIO_186"),
	PINCTRL_PIN(187, "GPIO_187"),
	PINCTRL_PIN(188, "GPIO_188"),
	PINCTRL_PIN(189, "GPIO_189"),
	PINCTRL_PIN(190, "GPIO_190"),
	PINCTRL_PIN(191, "GPIO_191"),
	PINCTRL_PIN(192, "GPIO_192"),
	PINCTRL_PIN(193, "GPIO_193"),
	PINCTRL_PIN(194, "GPIO_194"),
	PINCTRL_PIN(195, "GPIO_195"),
	PINCTRL_PIN(196, "GPIO_196"),
	PINCTRL_PIN(197, "GPIO_197"),
	PINCTRL_PIN(198, "GPIO_198"),
	PINCTRL_PIN(199, "GPIO_199"),
	PINCTRL_PIN(200, "GPIO_200"),
	PINCTRL_PIN(201, "GPIO_201"),
	PINCTRL_PIN(202, "GPIO_202"),
	PINCTRL_PIN(203, "GPIO_203"),
	PINCTRL_PIN(204, "GPIO_204"),
	PINCTRL_PIN(205, "GPIO_205"),
	PINCTRL_PIN(206, "GPIO_206"),
	PINCTRL_PIN(207, "GPIO_207"),
	PINCTRL_PIN(208, "GPIO_208"),
	PINCTRL_PIN(209, "GPIO_209"),
	PINCTRL_PIN(210, "GPIO_210"),
	PINCTRL_PIN(211, "GPIO_211"),
	PINCTRL_PIN(212, "GPIO_212"),
	PINCTRL_PIN(213, "GPIO_213"),
	PINCTRL_PIN(214, "GPIO_214"),
	PINCTRL_PIN(215, "GPIO_215"),
	PINCTRL_PIN(216, "GPIO_216"),
	PINCTRL_PIN(217, "GPIO_217"),
	PINCTRL_PIN(218, "GPIO_218"),
	PINCTRL_PIN(219, "GPIO_219"),
	PINCTRL_PIN(220, "GPIO_220"),
	PINCTRL_PIN(221, "GPIO_221"),
	PINCTRL_PIN(222, "GPIO_222"),
	PINCTRL_PIN(223, "GPIO_223"),
	PINCTRL_PIN(224, "UFS_RESET"),
	PINCTRL_PIN(225, "SDC2_CLK"),
	PINCTRL_PIN(226, "SDC2_CMD"),
	PINCTRL_PIN(227, "SDC2_DATA"),
};

#define DECLARE_MSM_GPIO_PINS(pin) \
	static const unsigned int gpio##pin##_pins[] = { pin }
DECLARE_MSM_GPIO_PINS(0);
DECLARE_MSM_GPIO_PINS(1);
DECLARE_MSM_GPIO_PINS(2);
DECLARE_MSM_GPIO_PINS(3);
DECLARE_MSM_GPIO_PINS(4);
DECLARE_MSM_GPIO_PINS(5);
DECLARE_MSM_GPIO_PINS(6);
DECLARE_MSM_GPIO_PINS(7);
DECLARE_MSM_GPIO_PINS(8);
DECLARE_MSM_GPIO_PINS(9);
DECLARE_MSM_GPIO_PINS(10);
DECLARE_MSM_GPIO_PINS(11);
DECLARE_MSM_GPIO_PINS(12);
DECLARE_MSM_GPIO_PINS(13);
DECLARE_MSM_GPIO_PINS(14);
DECLARE_MSM_GPIO_PINS(15);
DECLARE_MSM_GPIO_PINS(16);
DECLARE_MSM_GPIO_PINS(17);
DECLARE_MSM_GPIO_PINS(18);
DECLARE_MSM_GPIO_PINS(19);
DECLARE_MSM_GPIO_PINS(20);
DECLARE_MSM_GPIO_PINS(21);
DECLARE_MSM_GPIO_PINS(22);
DECLARE_MSM_GPIO_PINS(23);
DECLARE_MSM_GPIO_PINS(24);
DECLARE_MSM_GPIO_PINS(25);
DECLARE_MSM_GPIO_PINS(26);
DECLARE_MSM_GPIO_PINS(27);
DECLARE_MSM_GPIO_PINS(28);
DECLARE_MSM_GPIO_PINS(29);
DECLARE_MSM_GPIO_PINS(30);
DECLARE_MSM_GPIO_PINS(31);
DECLARE_MSM_GPIO_PINS(32);
DECLARE_MSM_GPIO_PINS(33);
DECLARE_MSM_GPIO_PINS(34);
DECLARE_MSM_GPIO_PINS(35);
DECLARE_MSM_GPIO_PINS(36);
DECLARE_MSM_GPIO_PINS(37);
DECLARE_MSM_GPIO_PINS(38);
DECLARE_MSM_GPIO_PINS(39);
DECLARE_MSM_GPIO_PINS(40);
DECLARE_MSM_GPIO_PINS(41);
DECLARE_MSM_GPIO_PINS(42);
DECLARE_MSM_GPIO_PINS(43);
DECLARE_MSM_GPIO_PINS(44);
DECLARE_MSM_GPIO_PINS(45);
DECLARE_MSM_GPIO_PINS(46);
DECLARE_MSM_GPIO_PINS(47);
DECLARE_MSM_GPIO_PINS(48);
DECLARE_MSM_GPIO_PINS(49);
DECLARE_MSM_GPIO_PINS(50);
DECLARE_MSM_GPIO_PINS(51);
DECLARE_MSM_GPIO_PINS(52);
DECLARE_MSM_GPIO_PINS(53);
DECLARE_MSM_GPIO_PINS(54);
DECLARE_MSM_GPIO_PINS(55);
DECLARE_MSM_GPIO_PINS(56);
DECLARE_MSM_GPIO_PINS(57);
DECLARE_MSM_GPIO_PINS(58);
DECLARE_MSM_GPIO_PINS(59);
DECLARE_MSM_GPIO_PINS(60);
DECLARE_MSM_GPIO_PINS(61);
DECLARE_MSM_GPIO_PINS(62);
DECLARE_MSM_GPIO_PINS(63);
DECLARE_MSM_GPIO_PINS(64);
DECLARE_MSM_GPIO_PINS(65);
DECLARE_MSM_GPIO_PINS(66);
DECLARE_MSM_GPIO_PINS(67);
DECLARE_MSM_GPIO_PINS(68);
DECLARE_MSM_GPIO_PINS(69);
DECLARE_MSM_GPIO_PINS(70);
DECLARE_MSM_GPIO_PINS(71);
DECLARE_MSM_GPIO_PINS(72);
DECLARE_MSM_GPIO_PINS(73);
DECLARE_MSM_GPIO_PINS(74);
DECLARE_MSM_GPIO_PINS(75);
DECLARE_MSM_GPIO_PINS(76);
DECLARE_MSM_GPIO_PINS(77);
DECLARE_MSM_GPIO_PINS(78);
DECLARE_MSM_GPIO_PINS(79);
DECLARE_MSM_GPIO_PINS(80);
DECLARE_MSM_GPIO_PINS(81);
DECLARE_MSM_GPIO_PINS(82);
DECLARE_MSM_GPIO_PINS(83);
DECLARE_MSM_GPIO_PINS(84);
DECLARE_MSM_GPIO_PINS(85);
DECLARE_MSM_GPIO_PINS(86);
DECLARE_MSM_GPIO_PINS(87);
DECLARE_MSM_GPIO_PINS(88);
DECLARE_MSM_GPIO_PINS(89);
DECLARE_MSM_GPIO_PINS(90);
DECLARE_MSM_GPIO_PINS(91);
DECLARE_MSM_GPIO_PINS(92);
DECLARE_MSM_GPIO_PINS(93);
DECLARE_MSM_GPIO_PINS(94);
DECLARE_MSM_GPIO_PINS(95);
DECLARE_MSM_GPIO_PINS(96);
DECLARE_MSM_GPIO_PINS(97);
DECLARE_MSM_GPIO_PINS(98);
DECLARE_MSM_GPIO_PINS(99);
DECLARE_MSM_GPIO_PINS(100);
DECLARE_MSM_GPIO_PINS(101);
DECLARE_MSM_GPIO_PINS(102);
DECLARE_MSM_GPIO_PINS(103);
DECLARE_MSM_GPIO_PINS(104);
DECLARE_MSM_GPIO_PINS(105);
DECLARE_MSM_GPIO_PINS(106);
DECLARE_MSM_GPIO_PINS(107);
DECLARE_MSM_GPIO_PINS(108);
DECLARE_MSM_GPIO_PINS(109);
DECLARE_MSM_GPIO_PINS(110);
DECLARE_MSM_GPIO_PINS(111);
DECLARE_MSM_GPIO_PINS(112);
DECLARE_MSM_GPIO_PINS(113);
DECLARE_MSM_GPIO_PINS(114);
DECLARE_MSM_GPIO_PINS(115);
DECLARE_MSM_GPIO_PINS(116);
DECLARE_MSM_GPIO_PINS(117);
DECLARE_MSM_GPIO_PINS(118);
DECLARE_MSM_GPIO_PINS(119);
DECLARE_MSM_GPIO_PINS(120);
DECLARE_MSM_GPIO_PINS(121);
DECLARE_MSM_GPIO_PINS(122);
DECLARE_MSM_GPIO_PINS(123);
DECLARE_MSM_GPIO_PINS(124);
DECLARE_MSM_GPIO_PINS(125);
DECLARE_MSM_GPIO_PINS(126);
DECLARE_MSM_GPIO_PINS(127);
DECLARE_MSM_GPIO_PINS(128);
DECLARE_MSM_GPIO_PINS(129);
DECLARE_MSM_GPIO_PINS(130);
DECLARE_MSM_GPIO_PINS(131);
DECLARE_MSM_GPIO_PINS(132);
DECLARE_MSM_GPIO_PINS(133);
DECLARE_MSM_GPIO_PINS(134);
DECLARE_MSM_GPIO_PINS(135);
DECLARE_MSM_GPIO_PINS(136);
DECLARE_MSM_GPIO_PINS(137);
DECLARE_MSM_GPIO_PINS(138);
DECLARE_MSM_GPIO_PINS(139);
DECLARE_MSM_GPIO_PINS(140);
DECLARE_MSM_GPIO_PINS(141);
DECLARE_MSM_GPIO_PINS(142);
DECLARE_MSM_GPIO_PINS(143);
DECLARE_MSM_GPIO_PINS(144);
DECLARE_MSM_GPIO_PINS(145);
DECLARE_MSM_GPIO_PINS(146);
DECLARE_MSM_GPIO_PINS(147);
DECLARE_MSM_GPIO_PINS(148);
DECLARE_MSM_GPIO_PINS(149);
DECLARE_MSM_GPIO_PINS(150);
DECLARE_MSM_GPIO_PINS(151);
DECLARE_MSM_GPIO_PINS(152);
DECLARE_MSM_GPIO_PINS(153);
DECLARE_MSM_GPIO_PINS(154);
DECLARE_MSM_GPIO_PINS(155);
DECLARE_MSM_GPIO_PINS(156);
DECLARE_MSM_GPIO_PINS(157);
DECLARE_MSM_GPIO_PINS(158);
DECLARE_MSM_GPIO_PINS(159);
DECLARE_MSM_GPIO_PINS(160);
DECLARE_MSM_GPIO_PINS(161);
DECLARE_MSM_GPIO_PINS(162);
DECLARE_MSM_GPIO_PINS(163);
DECLARE_MSM_GPIO_PINS(164);
DECLARE_MSM_GPIO_PINS(165);
DECLARE_MSM_GPIO_PINS(166);
DECLARE_MSM_GPIO_PINS(167);
DECLARE_MSM_GPIO_PINS(168);
DECLARE_MSM_GPIO_PINS(169);
DECLARE_MSM_GPIO_PINS(170);
DECLARE_MSM_GPIO_PINS(171);
DECLARE_MSM_GPIO_PINS(172);
DECLARE_MSM_GPIO_PINS(173);
DECLARE_MSM_GPIO_PINS(174);
DECLARE_MSM_GPIO_PINS(175);
DECLARE_MSM_GPIO_PINS(176);
DECLARE_MSM_GPIO_PINS(177);
DECLARE_MSM_GPIO_PINS(178);
DECLARE_MSM_GPIO_PINS(179);
DECLARE_MSM_GPIO_PINS(180);
DECLARE_MSM_GPIO_PINS(181);
DECLARE_MSM_GPIO_PINS(182);
DECLARE_MSM_GPIO_PINS(183);
DECLARE_MSM_GPIO_PINS(184);
DECLARE_MSM_GPIO_PINS(185);
DECLARE_MSM_GPIO_PINS(186);
DECLARE_MSM_GPIO_PINS(187);
DECLARE_MSM_GPIO_PINS(188);
DECLARE_MSM_GPIO_PINS(189);
DECLARE_MSM_GPIO_PINS(190);
DECLARE_MSM_GPIO_PINS(191);
DECLARE_MSM_GPIO_PINS(192);
DECLARE_MSM_GPIO_PINS(193);
DECLARE_MSM_GPIO_PINS(194);
DECLARE_MSM_GPIO_PINS(195);
DECLARE_MSM_GPIO_PINS(196);
DECLARE_MSM_GPIO_PINS(197);
DECLARE_MSM_GPIO_PINS(198);
DECLARE_MSM_GPIO_PINS(199);
DECLARE_MSM_GPIO_PINS(200);
DECLARE_MSM_GPIO_PINS(201);
DECLARE_MSM_GPIO_PINS(202);
DECLARE_MSM_GPIO_PINS(203);
DECLARE_MSM_GPIO_PINS(204);
DECLARE_MSM_GPIO_PINS(205);
DECLARE_MSM_GPIO_PINS(206);
DECLARE_MSM_GPIO_PINS(207);
DECLARE_MSM_GPIO_PINS(208);
DECLARE_MSM_GPIO_PINS(209);
DECLARE_MSM_GPIO_PINS(210);
DECLARE_MSM_GPIO_PINS(211);
DECLARE_MSM_GPIO_PINS(212);
DECLARE_MSM_GPIO_PINS(213);
DECLARE_MSM_GPIO_PINS(214);
DECLARE_MSM_GPIO_PINS(215);
DECLARE_MSM_GPIO_PINS(216);
DECLARE_MSM_GPIO_PINS(217);
DECLARE_MSM_GPIO_PINS(218);
DECLARE_MSM_GPIO_PINS(219);
DECLARE_MSM_GPIO_PINS(220);
DECLARE_MSM_GPIO_PINS(221);
DECLARE_MSM_GPIO_PINS(222);
DECLARE_MSM_GPIO_PINS(223);

static const unsigned int ufs_reset_pins[] = { 224 };
static const unsigned int sdc2_clk_pins[] = { 225 };
static const unsigned int sdc2_cmd_pins[] = { 226 };
static const unsigned int sdc2_data_pins[] = { 227 };

enum anorak_functions {
	msm_mux_gpio,
	msm_mux_atest_char,
	msm_mux_atest_char0,
	msm_mux_atest_char1,
	msm_mux_atest_char2,
	msm_mux_atest_char3,
	msm_mux_atest_usb0,
	msm_mux_atest_usb00,
	msm_mux_atest_usb01,
	msm_mux_atest_usb02,
	msm_mux_atest_usb03,
	msm_mux_audio_ref,
	msm_mux_cam_mclk,
	msm_mux_cci0_async,
	msm_mux_cci0_timer0,
	msm_mux_cci0_timer1,
	msm_mux_cci0_timer2,
	msm_mux_cci0_timer3,
	msm_mux_cci0_timer4,
	msm_mux_cci1_async,
	msm_mux_cci1_timer0,
	msm_mux_cci1_timer1,
	msm_mux_cci1_timer2,
	msm_mux_cci1_timer3,
	msm_mux_cci1_timer4,
	msm_mux_cci2_async,
	msm_mux_cci2_timer0,
	msm_mux_cci2_timer1,
	msm_mux_cci2_timer2,
	msm_mux_cci2_timer3,
	msm_mux_cci2_timer4,
	msm_mux_cci_i2c,
	msm_mux_cmu_rng0,
	msm_mux_cmu_rng1,
	msm_mux_cmu_rng2,
	msm_mux_cmu_rng3,
	msm_mux_dbg_out,
	msm_mux_ddr_bist,
	msm_mux_ddr_pxi0,
	msm_mux_ddr_pxi1,
	msm_mux_ddr_pxi2,
	msm_mux_ddr_pxi3,
	msm_mux_dp0_hot,
	msm_mux_edp0_hot,
	msm_mux_edp0_lcd,
	msm_mux_edp1_dpu0,
	msm_mux_edp1_dpu1,
	msm_mux_edp1_hot,
	msm_mux_ext_mclk0,
	msm_mux_ext_mclk1,
	msm_mux_gcc_gp1,
	msm_mux_gcc_gp10,
	msm_mux_gcc_gp11,
	msm_mux_gcc_gp2,
	msm_mux_gcc_gp3,
	msm_mux_gcc_gp4,
	msm_mux_gcc_gp5,
	msm_mux_gcc_gp6,
	msm_mux_gcc_gp7,
	msm_mux_gcc_gp8,
	msm_mux_gcc_gp9,
	msm_mux_i2s0_data0,
	msm_mux_i2s0_data1,
	msm_mux_i2s0_sck,
	msm_mux_i2s0_ws,
	msm_mux_i2s2_data0,
	msm_mux_i2s2_data1,
	msm_mux_i2s2_sck,
	msm_mux_i2s2_ws,
	msm_mux_ibi_i3c,
	msm_mux_jitter_bist,
	msm_mux_mdp0_vsync0,
	msm_mux_mdp0_vsync1,
	msm_mux_mdp0_vsync2,
	msm_mux_mdp0_vsync3,
	msm_mux_mdp0_vsync4,
	msm_mux_mdp0_vsync5,
	msm_mux_mdp0_vsync6,
	msm_mux_mdp0_vsync7,
	msm_mux_mdp0_vsync8,
	msm_mux_mdp1_vsync0,
	msm_mux_mdp1_vsync1,
	msm_mux_mdp1_vsync2,
	msm_mux_mdp1_vsync3,
	msm_mux_mdp1_vsync4,
	msm_mux_mdp1_vsync5,
	msm_mux_mdp1_vsync6,
	msm_mux_mdp1_vsync7,
	msm_mux_mdp1_vsync8,
	msm_mux_pcie0_clk,
	msm_mux_pcie1_clk,
	msm_mux_pcie2_clk,
	msm_mux_phase_flag0,
	msm_mux_phase_flag1,
	msm_mux_phase_flag10,
	msm_mux_phase_flag11,
	msm_mux_phase_flag12,
	msm_mux_phase_flag13,
	msm_mux_phase_flag14,
	msm_mux_phase_flag15,
	msm_mux_phase_flag16,
	msm_mux_phase_flag17,
	msm_mux_phase_flag18,
	msm_mux_phase_flag19,
	msm_mux_phase_flag2,
	msm_mux_phase_flag20,
	msm_mux_phase_flag21,
	msm_mux_phase_flag22,
	msm_mux_phase_flag23,
	msm_mux_phase_flag24,
	msm_mux_phase_flag25,
	msm_mux_phase_flag26,
	msm_mux_phase_flag27,
	msm_mux_phase_flag28,
	msm_mux_phase_flag29,
	msm_mux_phase_flag3,
	msm_mux_phase_flag30,
	msm_mux_phase_flag31,
	msm_mux_phase_flag4,
	msm_mux_phase_flag5,
	msm_mux_phase_flag6,
	msm_mux_phase_flag7,
	msm_mux_phase_flag8,
	msm_mux_phase_flag9,
	msm_mux_pll_bist,
	msm_mux_pll_clk,
	msm_mux_prng_rosc0,
	msm_mux_prng_rosc1,
	msm_mux_prng_rosc2,
	msm_mux_prng_rosc3,
	msm_mux_pwm_0,
	msm_mux_pwm_1,
	msm_mux_pwm_10,
	msm_mux_pwm_11,
	msm_mux_pwm_12,
	msm_mux_pwm_13,
	msm_mux_pwm_14,
	msm_mux_pwm_15,
	msm_mux_pwm_16,
	msm_mux_pwm_17,
	msm_mux_pwm_18,
	msm_mux_pwm_19,
	msm_mux_pwm_2,
	msm_mux_pwm_3,
	msm_mux_pwm_4,
	msm_mux_pwm_5,
	msm_mux_pwm_6,
	msm_mux_pwm_7,
	msm_mux_pwm_8,
	msm_mux_pwm_9,
	msm_mux_qdss_cti,
	msm_mux_qdss_gpio,
	msm_mux_qdss_gpio0,
	msm_mux_qdss_gpio1,
	msm_mux_qdss_gpio10,
	msm_mux_qdss_gpio11,
	msm_mux_qdss_gpio12,
	msm_mux_qdss_gpio13,
	msm_mux_qdss_gpio14,
	msm_mux_qdss_gpio15,
	msm_mux_qdss_gpio2,
	msm_mux_qdss_gpio3,
	msm_mux_qdss_gpio4,
	msm_mux_qdss_gpio5,
	msm_mux_qdss_gpio6,
	msm_mux_qdss_gpio7,
	msm_mux_qdss_gpio8,
	msm_mux_qdss_gpio9,
	msm_mux_qup0_se0,
	msm_mux_qup0_se1,
	msm_mux_qup0_se2,
	msm_mux_qup0_se3,
	msm_mux_qup0_se4,
	msm_mux_qup0_se5,
	msm_mux_qup0_se6,
	msm_mux_qup1_se0,
	msm_mux_qup1_se1,
	msm_mux_qup1_se2,
	msm_mux_qup1_se3,
	msm_mux_qup1_se4,
	msm_mux_qup1_se5,
	msm_mux_qup1_se6,
	msm_mux_sd_write,
	msm_mux_sdcc5_vdd2,
	msm_mux_tb_trig,
	msm_mux_tgu_ch0,
	msm_mux_tgu_ch1,
	msm_mux_tgu_ch2,
	msm_mux_tgu_ch3,
	msm_mux_tmess_prng0,
	msm_mux_tmess_prng1,
	msm_mux_tmess_prng2,
	msm_mux_tmess_prng3,
	msm_mux_tsense_pwm1,
	msm_mux_tsense_pwm2,
	msm_mux_usb0_phy,
	msm_mux_usb2phy_ac,
	msm_mux_vsense_trigger,
	msm_mux_NA,
};

static const char * const gpio_groups[] = {
	"gpio0", "gpio1", "gpio2", "gpio3", "gpio4", "gpio5", "gpio6", "gpio7",
	"gpio8", "gpio9", "gpio10", "gpio11", "gpio12", "gpio13", "gpio14",
	"gpio15", "gpio16", "gpio17", "gpio18", "gpio19", "gpio20", "gpio21",
	"gpio22", "gpio23", "gpio24", "gpio25", "gpio26", "gpio27", "gpio28",
	"gpio29", "gpio30", "gpio31", "gpio32", "gpio33", "gpio34", "gpio35",
	"gpio36", "gpio37", "gpio38", "gpio39", "gpio40", "gpio41", "gpio42",
	"gpio43", "gpio44", "gpio45", "gpio46", "gpio47", "gpio48", "gpio49",
	"gpio50", "gpio51", "gpio52", "gpio53", "gpio54", "gpio55", "gpio56",
	"gpio57", "gpio58", "gpio59", "gpio60", "gpio61", "gpio62", "gpio63",
	"gpio64", "gpio65", "gpio66", "gpio67", "gpio68", "gpio69", "gpio70",
	"gpio71", "gpio72", "gpio73", "gpio74", "gpio75", "gpio76", "gpio77",
	"gpio78", "gpio79", "gpio80", "gpio81", "gpio82", "gpio83", "gpio84",
	"gpio85", "gpio86", "gpio87", "gpio88", "gpio89", "gpio90", "gpio91",
	"gpio92", "gpio93", "gpio94", "gpio95", "gpio96", "gpio97", "gpio98",
	"gpio99", "gpio100", "gpio101", "gpio102", "gpio103", "gpio104",
	"gpio105", "gpio106", "gpio107", "gpio108", "gpio109", "gpio110",
	"gpio111", "gpio112", "gpio113", "gpio114", "gpio115", "gpio116",
	"gpio117", "gpio118", "gpio119", "gpio120", "gpio121", "gpio122",
	"gpio123", "gpio124", "gpio125", "gpio126", "gpio127", "gpio128",
	"gpio129", "gpio130", "gpio131", "gpio132", "gpio133", "gpio134",
	"gpio135", "gpio136", "gpio137", "gpio138", "gpio139", "gpio140",
	"gpio141", "gpio142", "gpio143", "gpio144", "gpio145", "gpio146",
	"gpio147", "gpio148", "gpio149", "gpio150", "gpio151", "gpio152",
	"gpio153", "gpio154", "gpio155", "gpio156", "gpio157", "gpio158",
	"gpio159", "gpio160", "gpio161", "gpio162", "gpio163", "gpio164",
	"gpio165", "gpio166", "gpio167", "gpio168", "gpio169", "gpio170",
	"gpio171", "gpio172", "gpio173", "gpio174", "gpio175", "gpio176",
	"gpio177", "gpio178", "gpio179", "gpio180", "gpio181", "gpio182",
	"gpio183", "gpio184", "gpio185", "gpio186", "gpio187", "gpio188",
	"gpio189", "gpio190", "gpio191", "gpio192", "gpio193", "gpio194",
	"gpio195", "gpio196", "gpio197", "gpio198", "gpio199", "gpio200",
	"gpio201", "gpio202", "gpio203", "gpio204", "gpio205", "gpio206",
	"gpio207", "gpio208", "gpio209", "gpio210", "gpio211", "gpio212",
	"gpio213", "gpio214", "gpio215", "gpio216", "gpio217", "gpio218",
	"gpio219", "gpio220", "gpio221", "gpio222", "gpio223",
};
static const char * const atest_char_groups[] = {
	"gpio15",
};
static const char * const atest_char0_groups[] = {
	"gpio19",
};
static const char * const atest_char1_groups[] = {
	"gpio18",
};
static const char * const atest_char2_groups[] = {
	"gpio17",
};
static const char * const atest_char3_groups[] = {
	"gpio16",
};
static const char * const atest_usb0_groups[] = {
	"gpio126",
};
static const char * const atest_usb00_groups[] = {
	"gpio127",
};
static const char * const atest_usb01_groups[] = {
	"gpio128",
};
static const char * const atest_usb02_groups[] = {
	"gpio129",
};
static const char * const atest_usb03_groups[] = {
	"gpio130",
};
static const char * const audio_ref_groups[] = {
	"gpio162",
};
static const char * const cam_mclk_groups[] = {
	"gpio20", "gpio21", "gpio22", "gpio23", "gpio24", "gpio25", "gpio26",
	"gpio27", "gpio28", "gpio29", "gpio30", "gpio31",
};
static const char * const cci0_async_groups[] = {
	"gpio40", "gpio41", "gpio42",
};
static const char * const cci0_timer0_groups[] = {
	"gpio32",
};
static const char * const cci0_timer1_groups[] = {
	"gpio33",
};
static const char * const cci0_timer2_groups[] = {
	"gpio34",
};
static const char * const cci0_timer3_groups[] = {
	"gpio35",
};
static const char * const cci0_timer4_groups[] = {
	"gpio36",
};
static const char * const cci1_async_groups[] = {
	"gpio43", "gpio44", "gpio45",
};
static const char * const cci1_timer0_groups[] = {
	"gpio37",
};
static const char * const cci1_timer1_groups[] = {
	"gpio38",
};
static const char * const cci1_timer2_groups[] = {
	"gpio39",
};
static const char * const cci1_timer3_groups[] = {
	"gpio53",
};
static const char * const cci1_timer4_groups[] = {
	"gpio54",
};
static const char * const cci2_async_groups[] = {
	"gpio46", "gpio47", "gpio48",
};
static const char * const cci2_timer0_groups[] = {
	"gpio55",
};
static const char * const cci2_timer1_groups[] = {
	"gpio56",
};
static const char * const cci2_timer2_groups[] = {
	"gpio45", "gpio49",
};
static const char * const cci2_timer3_groups[] = {
	"gpio46", "gpio50",
};
static const char * const cci2_timer4_groups[] = {
	"gpio47", "gpio51",
};
static const char * const cci_i2c_groups[] = {
	"gpio0", "gpio1", "gpio2", "gpio3", "gpio4", "gpio5", "gpio6", "gpio7",
	"gpio8", "gpio9", "gpio10", "gpio11", "gpio12", "gpio13", "gpio14",
	"gpio15", "gpio16", "gpio17", "gpio18", "gpio19", "gpio49", "gpio50",
	"gpio51", "gpio52",
};
static const char * const cmu_rng0_groups[] = {
	"gpio131",
};
static const char * const cmu_rng1_groups[] = {
	"gpio130",
};
static const char * const cmu_rng2_groups[] = {
	"gpio129",
};
static const char * const cmu_rng3_groups[] = {
	"gpio128",
};
static const char * const dbg_out_groups[] = {
	"gpio133",
};
static const char * const ddr_bist_groups[] = {
	"gpio2", "gpio3", "gpio4", "gpio5",
};
static const char * const ddr_pxi0_groups[] = {
	"gpio126", "gpio127",
};
static const char * const ddr_pxi1_groups[] = {
	"gpio128", "gpio129",
};
static const char * const ddr_pxi2_groups[] = {
	"gpio130", "gpio131",
};
static const char * const ddr_pxi3_groups[] = {
	"gpio132", "gpio133",
};
static const char * const dp0_hot_groups[] = {
	"gpio103",
};
static const char * const edp0_hot_groups[] = {
	"gpio104", "gpio147",
};
static const char * const edp0_lcd_groups[] = {
	"gpio106",
};
static const char * const edp1_dpu0_groups[] = {
	"gpio107",
};
static const char * const edp1_dpu1_groups[] = {
	"gpio107",
};
static const char * const edp1_hot_groups[] = {
	"gpio105", "gpio148",
};
static const char * const ext_mclk0_groups[] = {
	"gpio162",
};
static const char * const ext_mclk1_groups[] = {
	"gpio116",
};
static const char * const gcc_gp1_groups[] = {
	"gpio48", "gpio138",
};
static const char * const gcc_gp10_groups[] = {
	"gpio108",
};
static const char * const gcc_gp11_groups[] = {
	"gpio109",
};
static const char * const gcc_gp2_groups[] = {
	"gpio53", "gpio139",
};
static const char * const gcc_gp3_groups[] = {
	"gpio54", "gpio140",
};
static const char * const gcc_gp4_groups[] = {
	"gpio62",
};
static const char * const gcc_gp5_groups[] = {
	"gpio68",
};
static const char * const gcc_gp6_groups[] = {
	"gpio74",
};
static const char * const gcc_gp7_groups[] = {
	"gpio80",
};
static const char * const gcc_gp8_groups[] = {
	"gpio87",
};
static const char * const gcc_gp9_groups[] = {
	"gpio107",
};
static const char * const i2s0_data0_groups[] = {
	"gpio163",
};
static const char * const i2s0_data1_groups[] = {
	"gpio165",
};
static const char * const i2s0_sck_groups[] = {
	"gpio164",
};
static const char * const i2s0_ws_groups[] = {
	"gpio166",
};
static const char * const i2s2_data0_groups[] = {
	"gpio155",
};
static const char * const i2s2_data1_groups[] = {
	"gpio154",
};
static const char * const i2s2_sck_groups[] = {
	"gpio156",
};
static const char * const i2s2_ws_groups[] = {
	"gpio157",
};
static const char * const ibi_i3c_groups[] = {
	"gpio57", "gpio58", "gpio63", "gpio64", "gpio94", "gpio95", "gpio98",
	"gpio99",
};
static const char * const jitter_bist_groups[] = {
	"gpio158",
};
static const char * const mdp0_vsync0_groups[] = {
	"gpio69", "gpio110", "gpio150",
};
static const char * const mdp0_vsync1_groups[] = {
	"gpio70", "gpio111", "gpio151",
};
static const char * const mdp0_vsync2_groups[] = {
	"gpio71",
};
static const char * const mdp0_vsync3_groups[] = {
	"gpio72",
};
static const char * const mdp0_vsync4_groups[] = {
	"gpio73",
};
static const char * const mdp0_vsync5_groups[] = {
	"gpio74",
};
static const char * const mdp0_vsync6_groups[] = {
	"gpio75",
};
static const char * const mdp0_vsync7_groups[] = {
	"gpio76",
};
static const char * const mdp0_vsync8_groups[] = {
	"gpio77",
};
static const char * const mdp1_vsync0_groups[] = {
	"gpio60", "gpio112", "gpio152",
};
static const char * const mdp1_vsync1_groups[] = {
	"gpio61", "gpio113", "gpio153",
};
static const char * const mdp1_vsync2_groups[] = {
	"gpio62",
};
static const char * const mdp1_vsync3_groups[] = {
	"gpio63",
};
static const char * const mdp1_vsync4_groups[] = {
	"gpio64",
};
static const char * const mdp1_vsync5_groups[] = {
	"gpio65",
};
static const char * const mdp1_vsync6_groups[] = {
	"gpio66",
};
static const char * const mdp1_vsync7_groups[] = {
	"gpio67",
};
static const char * const mdp1_vsync8_groups[] = {
	"gpio68",
};
static const char * const pcie0_clk_groups[] = {
	"gpio118",
};
static const char * const pcie1_clk_groups[] = {
	"gpio142",
};
static const char * const pcie2_clk_groups[] = {
	"gpio171",
};
static const char * const phase_flag0_groups[] = {
	"gpio11",
};
static const char * const phase_flag1_groups[] = {
	"gpio10",
};
static const char * const phase_flag10_groups[] = {
	"gpio41",
};
static const char * const phase_flag11_groups[] = {
	"gpio40",
};
static const char * const phase_flag12_groups[] = {
	"gpio101",
};
static const char * const phase_flag13_groups[] = {
	"gpio100",
};
static const char * const phase_flag14_groups[] = {
	"gpio97",
};
static const char * const phase_flag15_groups[] = {
	"gpio96",
};
static const char * const phase_flag16_groups[] = {
	"gpio39",
};
static const char * const phase_flag17_groups[] = {
	"gpio38",
};
static const char * const phase_flag18_groups[] = {
	"gpio37",
};
static const char * const phase_flag19_groups[] = {
	"gpio36",
};
static const char * const phase_flag2_groups[] = {
	"gpio9",
};
static const char * const phase_flag20_groups[] = {
	"gpio35",
};
static const char * const phase_flag21_groups[] = {
	"gpio34",
};
static const char * const phase_flag22_groups[] = {
	"gpio33",
};
static const char * const phase_flag23_groups[] = {
	"gpio32",
};
static const char * const phase_flag24_groups[] = {
	"gpio19",
};
static const char * const phase_flag25_groups[] = {
	"gpio18",
};
static const char * const phase_flag26_groups[] = {
	"gpio17",
};
static const char * const phase_flag27_groups[] = {
	"gpio16",
};
static const char * const phase_flag28_groups[] = {
	"gpio15",
};
static const char * const phase_flag29_groups[] = {
	"gpio14",
};
static const char * const phase_flag3_groups[] = {
	"gpio8",
};
static const char * const phase_flag30_groups[] = {
	"gpio13",
};
static const char * const phase_flag31_groups[] = {
	"gpio12",
};
static const char * const phase_flag4_groups[] = {
	"gpio7",
};
static const char * const phase_flag5_groups[] = {
	"gpio6",
};
static const char * const phase_flag6_groups[] = {
	"gpio5",
};
static const char * const phase_flag7_groups[] = {
	"gpio4",
};
static const char * const phase_flag8_groups[] = {
	"gpio3",
};
static const char * const phase_flag9_groups[] = {
	"gpio2",
};
static const char * const pll_bist_groups[] = {
	"gpio157",
};
static const char * const pll_clk_groups[] = {
	"gpio159",
};
static const char * const prng_rosc0_groups[] = {
	"gpio29",
};
static const char * const prng_rosc1_groups[] = {
	"gpio30",
};
static const char * const prng_rosc2_groups[] = {
	"gpio28",
};
static const char * const prng_rosc3_groups[] = {
	"gpio27",
};
static const char * const pwm_0_groups[] = {
	"gpio100",
};
static const char * const pwm_1_groups[] = {
	"gpio101",
};
static const char * const pwm_10_groups[] = {
	"gpio84",
};
static const char * const pwm_11_groups[] = {
	"gpio108",
};
static const char * const pwm_12_groups[] = {
	"gpio109",
};
static const char * const pwm_13_groups[] = {
	"gpio19",
};
static const char * const pwm_14_groups[] = {
	"gpio31",
};
static const char * const pwm_15_groups[] = {
	"gpio52",
};
static const char * const pwm_16_groups[] = {
	"gpio114",
};
static const char * const pwm_17_groups[] = {
	"gpio149",
};
static const char * const pwm_18_groups[] = {
	"gpio169",
};
static const char * const pwm_19_groups[] = {
	"gpio173",
};
static const char * const pwm_2_groups[] = {
	"gpio60",
};
static const char * const pwm_3_groups[] = {
	"gpio61",
};
static const char * const pwm_4_groups[] = {
	"gpio66",
};
static const char * const pwm_5_groups[] = {
	"gpio67",
};
static const char * const pwm_6_groups[] = {
	"gpio72",
};
static const char * const pwm_7_groups[] = {
	"gpio73",
};
static const char * const pwm_8_groups[] = {
	"gpio78",
};
static const char * const pwm_9_groups[] = {
	"gpio79",
};
static const char * const qdss_cti_groups[] = {
	"gpio42", "gpio43", "gpio114", "gpio115", "gpio154", "gpio155",
	"gpio156", "gpio157",
};
static const char * const qdss_gpio_groups[] = {
	"gpio28", "gpio29", "gpio202", "gpio203",
};
static const char * const qdss_gpio0_groups[] = {
	"gpio20", "gpio221",
};
static const char * const qdss_gpio1_groups[] = {
	"gpio21", "gpio223",
};
static const char * const qdss_gpio10_groups[] = {
	"gpio44", "gpio134",
};
static const char * const qdss_gpio11_groups[] = {
	"gpio45", "gpio215",
};
static const char * const qdss_gpio12_groups[] = {
	"gpio46", "gpio216",
};
static const char * const qdss_gpio13_groups[] = {
	"gpio47", "gpio217",
};
static const char * const qdss_gpio14_groups[] = {
	"gpio48", "gpio219",
};
static const char * const qdss_gpio15_groups[] = {
	"gpio49", "gpio135",
};
static const char * const qdss_gpio2_groups[] = {
	"gpio22", "gpio191",
};
static const char * const qdss_gpio3_groups[] = {
	"gpio23", "gpio192",
};
static const char * const qdss_gpio4_groups[] = {
	"gpio24", "gpio193",
};
static const char * const qdss_gpio5_groups[] = {
	"gpio25", "gpio194",
};
static const char * const qdss_gpio6_groups[] = {
	"gpio26", "gpio197",
};
static const char * const qdss_gpio7_groups[] = {
	"gpio27", "gpio198",
};
static const char * const qdss_gpio8_groups[] = {
	"gpio30", "gpio206",
};
static const char * const qdss_gpio9_groups[] = {
	"gpio31", "gpio207",
};
static const char * const qup0_se0_groups[] = {
	"gpio94", "gpio95", "gpio96", "gpio97",
};
static const char * const qup0_se1_groups[] = {
	"gpio98", "gpio99", "gpio100", "gpio101",
};
static const char * const qup0_se2_groups[] = {
	"gpio150", "gpio151", "gpio152", "gpio153",
};
static const char * const qup0_se3_groups[] = {
	"gpio134", "gpio135", "gpio136", "gpio137",
};
static const char * const qup0_se4_groups[] = {
	"gpio130", "gpio131", "gpio132", "gpio133", "gpio162", "gpio163",
	"gpio164", "gpio165", "gpio166", "gpio167", "gpio168",
};
static const char * const qup0_se5_groups[] = {
	"gpio126", "gpio127", "gpio128", "gpio129",
};
static const char * const qup0_se6_groups[] = {
	"gpio156", "gpio157", "gpio158", "gpio159",
};
static const char * const qup1_se0_groups[] = {
	"gpio57", "gpio58", "gpio59", "gpio60", "gpio62",
};
static const char * const qup1_se1_groups[] = {
	"gpio63", "gpio64", "gpio65", "gpio66", "gpio68",
};
static const char * const qup1_se2_groups[] = {
	"gpio69", "gpio70", "gpio71", "gpio72", "gpio74",
};
static const char * const qup1_se3_groups[] = {
	"gpio75", "gpio76", "gpio77", "gpio78", "gpio80",
};
static const char * const qup1_se4_groups[] = {
	"gpio81", "gpio82", "gpio83", "gpio84", "gpio85", "gpio86", "gpio87",
};
static const char * const qup1_se5_groups[] = {
	"gpio88", "gpio89", "gpio90", "gpio91",
};
static const char * const qup1_se6_groups[] = {
	"gpio92", "gpio93", "gpio103", "gpio104", "gpio105", "gpio106",
};
static const char * const sd_write_groups[] = {
	"gpio155",
};
static const char * const sdcc5_vdd2_groups[] = {
	"gpio154",
};
static const char * const tb_trig_groups[] = {
	"gpio174",
};
static const char * const tgu_ch0_groups[] = {
	"gpio163",
};
static const char * const tgu_ch1_groups[] = {
	"gpio164",
};
static const char * const tgu_ch2_groups[] = {
	"gpio165",
};
static const char * const tgu_ch3_groups[] = {
	"gpio166",
};
static const char * const tmess_prng0_groups[] = {
	"gpio26",
};
static const char * const tmess_prng1_groups[] = {
	"gpio25",
};
static const char * const tmess_prng2_groups[] = {
	"gpio24",
};
static const char * const tmess_prng3_groups[] = {
	"gpio23",
};
static const char * const tsense_pwm1_groups[] = {
	"gpio102",
};
static const char * const tsense_pwm2_groups[] = {
	"gpio102",
};
static const char * const usb0_phy_groups[] = {
	"gpio160",
};
static const char * const usb2phy_ac_groups[] = {
	"gpio141",
};
static const char * const vsense_trigger_groups[] = {
	"gpio126",
};

static const struct msm_function anorak_functions[] = {
	FUNCTION(gpio),
	FUNCTION(atest_char),
	FUNCTION(atest_char0),
	FUNCTION(atest_char1),
	FUNCTION(atest_char2),
	FUNCTION(atest_char3),
	FUNCTION(atest_usb0),
	FUNCTION(atest_usb00),
	FUNCTION(atest_usb01),
	FUNCTION(atest_usb02),
	FUNCTION(atest_usb03),
	FUNCTION(audio_ref),
	FUNCTION(cam_mclk),
	FUNCTION(cci0_async),
	FUNCTION(cci0_timer0),
	FUNCTION(cci0_timer1),
	FUNCTION(cci0_timer2),
	FUNCTION(cci0_timer3),
	FUNCTION(cci0_timer4),
	FUNCTION(cci1_async),
	FUNCTION(cci1_timer0),
	FUNCTION(cci1_timer1),
	FUNCTION(cci1_timer2),
	FUNCTION(cci1_timer3),
	FUNCTION(cci1_timer4),
	FUNCTION(cci2_async),
	FUNCTION(cci2_timer0),
	FUNCTION(cci2_timer1),
	FUNCTION(cci2_timer2),
	FUNCTION(cci2_timer3),
	FUNCTION(cci2_timer4),
	FUNCTION(cci_i2c),
	FUNCTION(cmu_rng0),
	FUNCTION(cmu_rng1),
	FUNCTION(cmu_rng2),
	FUNCTION(cmu_rng3),
	FUNCTION(dbg_out),
	FUNCTION(ddr_bist),
	FUNCTION(ddr_pxi0),
	FUNCTION(ddr_pxi1),
	FUNCTION(ddr_pxi2),
	FUNCTION(ddr_pxi3),
	FUNCTION(dp0_hot),
	FUNCTION(edp0_hot),
	FUNCTION(edp0_lcd),
	FUNCTION(edp1_dpu0),
	FUNCTION(edp1_dpu1),
	FUNCTION(edp1_hot),
	FUNCTION(ext_mclk0),
	FUNCTION(ext_mclk1),
	FUNCTION(gcc_gp1),
	FUNCTION(gcc_gp10),
	FUNCTION(gcc_gp11),
	FUNCTION(gcc_gp2),
	FUNCTION(gcc_gp3),
	FUNCTION(gcc_gp4),
	FUNCTION(gcc_gp5),
	FUNCTION(gcc_gp6),
	FUNCTION(gcc_gp7),
	FUNCTION(gcc_gp8),
	FUNCTION(gcc_gp9),
	FUNCTION(i2s0_data0),
	FUNCTION(i2s0_data1),
	FUNCTION(i2s0_sck),
	FUNCTION(i2s0_ws),
	FUNCTION(i2s2_data0),
	FUNCTION(i2s2_data1),
	FUNCTION(i2s2_sck),
	FUNCTION(i2s2_ws),
	FUNCTION(ibi_i3c),
	FUNCTION(jitter_bist),
	FUNCTION(mdp0_vsync0),
	FUNCTION(mdp0_vsync1),
	FUNCTION(mdp0_vsync2),
	FUNCTION(mdp0_vsync3),
	FUNCTION(mdp0_vsync4),
	FUNCTION(mdp0_vsync5),
	FUNCTION(mdp0_vsync6),
	FUNCTION(mdp0_vsync7),
	FUNCTION(mdp0_vsync8),
	FUNCTION(mdp1_vsync0),
	FUNCTION(mdp1_vsync1),
	FUNCTION(mdp1_vsync2),
	FUNCTION(mdp1_vsync3),
	FUNCTION(mdp1_vsync4),
	FUNCTION(mdp1_vsync5),
	FUNCTION(mdp1_vsync6),
	FUNCTION(mdp1_vsync7),
	FUNCTION(mdp1_vsync8),
	FUNCTION(pcie0_clk),
	FUNCTION(pcie1_clk),
	FUNCTION(pcie2_clk),
	FUNCTION(phase_flag0),
	FUNCTION(phase_flag1),
	FUNCTION(phase_flag10),
	FUNCTION(phase_flag11),
	FUNCTION(phase_flag12),
	FUNCTION(phase_flag13),
	FUNCTION(phase_flag14),
	FUNCTION(phase_flag15),
	FUNCTION(phase_flag16),
	FUNCTION(phase_flag17),
	FUNCTION(phase_flag18),
	FUNCTION(phase_flag19),
	FUNCTION(phase_flag2),
	FUNCTION(phase_flag20),
	FUNCTION(phase_flag21),
	FUNCTION(phase_flag22),
	FUNCTION(phase_flag23),
	FUNCTION(phase_flag24),
	FUNCTION(phase_flag25),
	FUNCTION(phase_flag26),
	FUNCTION(phase_flag27),
	FUNCTION(phase_flag28),
	FUNCTION(phase_flag29),
	FUNCTION(phase_flag3),
	FUNCTION(phase_flag30),
	FUNCTION(phase_flag31),
	FUNCTION(phase_flag4),
	FUNCTION(phase_flag5),
	FUNCTION(phase_flag6),
	FUNCTION(phase_flag7),
	FUNCTION(phase_flag8),
	FUNCTION(phase_flag9),
	FUNCTION(pll_bist),
	FUNCTION(pll_clk),
	FUNCTION(prng_rosc0),
	FUNCTION(prng_rosc1),
	FUNCTION(prng_rosc2),
	FUNCTION(prng_rosc3),
	FUNCTION(pwm_0),
	FUNCTION(pwm_1),
	FUNCTION(pwm_10),
	FUNCTION(pwm_11),
	FUNCTION(pwm_12),
	FUNCTION(pwm_13),
	FUNCTION(pwm_14),
	FUNCTION(pwm_15),
	FUNCTION(pwm_16),
	FUNCTION(pwm_17),
	FUNCTION(pwm_18),
	FUNCTION(pwm_19),
	FUNCTION(pwm_2),
	FUNCTION(pwm_3),
	FUNCTION(pwm_4),
	FUNCTION(pwm_5),
	FUNCTION(pwm_6),
	FUNCTION(pwm_7),
	FUNCTION(pwm_8),
	FUNCTION(pwm_9),
	FUNCTION(qdss_cti),
	FUNCTION(qdss_gpio),
	FUNCTION(qdss_gpio0),
	FUNCTION(qdss_gpio1),
	FUNCTION(qdss_gpio10),
	FUNCTION(qdss_gpio11),
	FUNCTION(qdss_gpio12),
	FUNCTION(qdss_gpio13),
	FUNCTION(qdss_gpio14),
	FUNCTION(qdss_gpio15),
	FUNCTION(qdss_gpio2),
	FUNCTION(qdss_gpio3),
	FUNCTION(qdss_gpio4),
	FUNCTION(qdss_gpio5),
	FUNCTION(qdss_gpio6),
	FUNCTION(qdss_gpio7),
	FUNCTION(qdss_gpio8),
	FUNCTION(qdss_gpio9),
	FUNCTION(qup0_se0),
	FUNCTION(qup0_se1),
	FUNCTION(qup0_se2),
	FUNCTION(qup0_se3),
	FUNCTION(qup0_se4),
	FUNCTION(qup0_se5),
	FUNCTION(qup0_se6),
	FUNCTION(qup1_se0),
	FUNCTION(qup1_se1),
	FUNCTION(qup1_se2),
	FUNCTION(qup1_se3),
	FUNCTION(qup1_se4),
	FUNCTION(qup1_se5),
	FUNCTION(qup1_se6),
	FUNCTION(sd_write),
	FUNCTION(sdcc5_vdd2),
	FUNCTION(tb_trig),
	FUNCTION(tgu_ch0),
	FUNCTION(tgu_ch1),
	FUNCTION(tgu_ch2),
	FUNCTION(tgu_ch3),
	FUNCTION(tmess_prng0),
	FUNCTION(tmess_prng1),
	FUNCTION(tmess_prng2),
	FUNCTION(tmess_prng3),
	FUNCTION(tsense_pwm1),
	FUNCTION(tsense_pwm2),
	FUNCTION(usb0_phy),
	FUNCTION(usb2phy_ac),
	FUNCTION(vsense_trigger),
};

/* Every pin is maintained as a single group, and missing or non-existing pin
 * would be maintained as dummy group to synchronize pin group index with
 * pin descriptor registered with pinctrl core.
 * Clients would not be able to request these dummy pin groups.
 */
static const struct msm_pingroup anorak_groups[] = {
	[0] = PINGROUP(0, cci_i2c, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[1] = PINGROUP(1, cci_i2c, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[2] = PINGROUP(2, cci_i2c, ddr_bist, NA, phase_flag9, NA, NA, NA, NA,
		       NA, 0, -1),
	[3] = PINGROUP(3, cci_i2c, ddr_bist, NA, phase_flag8, NA, NA, NA, NA,
		       NA, 0, -1),
	[4] = PINGROUP(4, cci_i2c, ddr_bist, NA, phase_flag7, NA, NA, NA, NA,
		       NA, 0, -1),
	[5] = PINGROUP(5, cci_i2c, ddr_bist, NA, phase_flag6, NA, NA, NA, NA,
		       NA, 0, -1),
	[6] = PINGROUP(6, cci_i2c, NA, phase_flag5, NA, NA, NA, NA, NA, NA, 0, -1),
	[7] = PINGROUP(7, cci_i2c, NA, phase_flag4, NA, NA, NA, NA, NA, NA, 0, -1),
	[8] = PINGROUP(8, cci_i2c, NA, phase_flag3, NA, NA, NA, NA, NA, NA, 0, -1),
	[9] = PINGROUP(9, cci_i2c, NA, phase_flag2, NA, NA, NA, NA, NA, NA, 0, -1),
	[10] = PINGROUP(10, cci_i2c, NA, phase_flag1, NA, NA, NA, NA, NA, NA, 0, -1),
	[11] = PINGROUP(11, cci_i2c, NA, phase_flag0, NA, NA, NA, NA, NA, NA, 0, -1),
	[12] = PINGROUP(12, cci_i2c, NA, phase_flag31, NA, NA, NA, NA, NA, NA, 0, -1),
	[13] = PINGROUP(13, cci_i2c, NA, phase_flag30, NA, NA, NA, NA, NA, NA, 0, -1),
	[14] = PINGROUP(14, cci_i2c, NA, phase_flag29, NA, NA, NA, NA, NA, NA, 0, -1),
	[15] = PINGROUP(15, cci_i2c, NA, phase_flag28, atest_char, NA, NA, NA,
			NA, NA, 0, -1),
	[16] = PINGROUP(16, cci_i2c, NA, phase_flag27, atest_char3, NA, NA, NA,
			NA, NA, 0, -1),
	[17] = PINGROUP(17, cci_i2c, NA, phase_flag26, atest_char2, NA, NA, NA,
			NA, NA, 0, -1),
	[18] = PINGROUP(18, cci_i2c, NA, phase_flag25, atest_char1, NA, NA, NA,
			NA, NA, 0xE000C, 0),
	[19] = PINGROUP(19, cci_i2c, pwm_13, NA, phase_flag24, atest_char0, NA,
			NA, NA, NA, 0, -1),
	[20] = PINGROUP(20, cam_mclk, qdss_gpio0, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[21] = PINGROUP(21, cam_mclk, qdss_gpio1, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[22] = PINGROUP(22, cam_mclk, qdss_gpio2, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[23] = PINGROUP(23, cam_mclk, tmess_prng3, qdss_gpio3, NA, NA, NA, NA,
			NA, NA, 0, -1),
	[24] = PINGROUP(24, cam_mclk, tmess_prng2, qdss_gpio4, NA, NA, NA, NA,
			NA, NA, 0, -1),
	[25] = PINGROUP(25, cam_mclk, tmess_prng1, qdss_gpio5, NA, NA, NA, NA,
			NA, NA, 0, -1),
	[26] = PINGROUP(26, cam_mclk, tmess_prng0, qdss_gpio6, NA, NA, NA, NA,
			NA, NA, 0, -1),
	[27] = PINGROUP(27, cam_mclk, prng_rosc3, qdss_gpio7, NA, NA, NA, NA,
			NA, NA, 0, -1),
	[28] = PINGROUP(28, cam_mclk, prng_rosc2, qdss_gpio, NA, NA, NA, NA,
			NA, NA, 0, -1),
	[29] = PINGROUP(29, cam_mclk, prng_rosc0, qdss_gpio, NA, NA, NA, NA,
			NA, NA, 0, -1),
	[30] = PINGROUP(30, cam_mclk, prng_rosc1, qdss_gpio8, NA, NA, NA, NA,
			NA, NA, 0, -1),
	[31] = PINGROUP(31, cam_mclk, pwm_14, qdss_gpio9, NA, NA, NA, NA, NA,
			NA, 0xE000C, 1),
	[32] = PINGROUP(32, cci0_timer0, NA, phase_flag23, NA, NA, NA, NA, NA,
			NA, 0, -1),
	[33] = PINGROUP(33, cci0_timer1, NA, phase_flag22, NA, NA, NA, NA, NA,
			NA, 0, -1),
	[34] = PINGROUP(34, cci0_timer2, NA, phase_flag21, NA, NA, NA, NA, NA,
			NA, 0, -1),
	[35] = PINGROUP(35, cci0_timer3, NA, NA, phase_flag20, NA, NA, NA, NA,
			NA, 0, -1),
	[36] = PINGROUP(36, cci0_timer4, NA, NA, phase_flag19, NA, NA, NA, NA,
			NA, 0, -1),
	[37] = PINGROUP(37, cci1_timer0, NA, NA, phase_flag18, NA, NA, NA, NA,
			NA, 0xE000C, 2),
	[38] = PINGROUP(38, cci1_timer1, NA, phase_flag17, NA, NA, NA, NA, NA,
			NA, 0, -1),
	[39] = PINGROUP(39, cci1_timer2, NA, phase_flag16, NA, NA, NA, NA, NA,
			NA, 0, -1),
	[40] = PINGROUP(40, cci0_async, NA, phase_flag11, NA, NA, NA, NA, NA,
			NA, 0xE000C, 3),
	[41] = PINGROUP(41, cci0_async, NA, phase_flag10, NA, NA, NA, NA, NA,
			NA, 0xE000C, 4),
	[42] = PINGROUP(42, cci0_async, qdss_cti, NA, NA, NA, NA, NA, NA, NA, 0xE000C, 5),
	[43] = PINGROUP(43, cci1_async, qdss_cti, NA, NA, NA, NA, NA, NA, NA, 0xE000C, 6),
	[44] = PINGROUP(44, cci1_async, qdss_gpio10, NA, NA, NA, NA, NA, NA,
			NA, 0xE000C, 7),
	[45] = PINGROUP(45, cci1_async, cci2_timer2, qdss_gpio11, NA, NA, NA,
			NA, NA, NA, 0xE000C, 8),
	[46] = PINGROUP(46, cci2_async, cci2_timer3, qdss_gpio12, NA, NA, NA,
			NA, NA, NA, 0xE000C, 9),
	[47] = PINGROUP(47, cci2_async, cci2_timer4, qdss_gpio13, NA, NA, NA,
			NA, NA, NA, 0xE000C, 10),
	[48] = PINGROUP(48, cci2_async, gcc_gp1, qdss_gpio14, NA, NA, NA, NA,
			NA, NA, 0xE000C, 11),
	[49] = PINGROUP(49, cci_i2c, cci2_timer2, qdss_gpio15, NA, NA, NA, NA,
			NA, NA, 0, -1),
	[50] = PINGROUP(50, cci_i2c, cci2_timer3, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[51] = PINGROUP(51, cci_i2c, cci2_timer4, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[52] = PINGROUP(52, cci_i2c, pwm_15, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[53] = PINGROUP(53, cci1_timer3, gcc_gp2, NA, NA, NA, NA, NA, NA, NA, 0xE000C, 12),
	[54] = PINGROUP(54, cci1_timer4, gcc_gp3, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[55] = PINGROUP(55, cci2_timer0, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[56] = PINGROUP(56, cci2_timer1, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[57] = PINGROUP(57, qup1_se0, ibi_i3c, NA, NA, NA, NA, NA, NA, NA, 0xE0010, 0),
	[58] = PINGROUP(58, qup1_se0, ibi_i3c, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[59] = PINGROUP(59, qup1_se0, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[60] = PINGROUP(60, qup1_se0, pwm_2, mdp1_vsync0, NA, NA, NA, NA, NA,
			NA, 0xE0010, 1),
	[61] = PINGROUP(61, pwm_3, mdp1_vsync1, NA, NA, NA, NA, NA, NA, NA, 0xE0010, 2),
	[62] = PINGROUP(62, gcc_gp4, qup1_se0, mdp1_vsync2, NA, NA, NA, NA, NA,
			NA, 0xE0010, 3),
	[63] = PINGROUP(63, qup1_se1, ibi_i3c, mdp1_vsync3, NA, NA, NA, NA, NA,
			NA, 0xE0010, 4),
	[64] = PINGROUP(64, qup1_se1, ibi_i3c, mdp1_vsync4, NA, NA, NA, NA, NA,
			NA, 0, -1),
	[65] = PINGROUP(65, qup1_se1, mdp1_vsync5, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[66] = PINGROUP(66, qup1_se1, pwm_4, mdp1_vsync6, NA, NA, NA, NA, NA,
			NA, 0xE0010, 5),
	[67] = PINGROUP(67, pwm_5, mdp1_vsync7, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[68] = PINGROUP(68, gcc_gp5, qup1_se1, mdp1_vsync8, NA, NA, NA, NA, NA,
			NA, 0xE0010, 6),
	[69] = PINGROUP(69, qup1_se2, mdp0_vsync0, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[70] = PINGROUP(70, qup1_se2, mdp0_vsync1, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[71] = PINGROUP(71, qup1_se2, mdp0_vsync2, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[72] = PINGROUP(72, qup1_se2, pwm_6, mdp0_vsync3, NA, NA, NA, NA, NA,
			NA, 0xE0010, 7),
	[73] = PINGROUP(73, pwm_7, mdp0_vsync4, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[74] = PINGROUP(74, gcc_gp6, qup1_se2, mdp0_vsync5, NA, NA, NA, NA, NA,
			NA, 0xE0010, 8),
	[75] = PINGROUP(75, qup1_se3, mdp0_vsync6, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[76] = PINGROUP(76, qup1_se3, mdp0_vsync7, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[77] = PINGROUP(77, qup1_se3, mdp0_vsync8, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[78] = PINGROUP(78, qup1_se3, pwm_8, NA, NA, NA, NA, NA, NA, NA, 0xE0010, 9),
	[79] = PINGROUP(79, pwm_9, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[80] = PINGROUP(80, gcc_gp7, qup1_se3, NA, NA, NA, NA, NA, NA, NA, 0xE0010, 10),
	[81] = PINGROUP(81, qup1_se4, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[82] = PINGROUP(82, qup1_se4, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[83] = PINGROUP(83, qup1_se4, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[84] = PINGROUP(84, qup1_se4, pwm_10, NA, NA, NA, NA, NA, NA, NA, 0xE0010, 11),
	[85] = PINGROUP(85, qup1_se4, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[86] = PINGROUP(86, qup1_se4, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0010, 12),
	[87] = PINGROUP(87, gcc_gp8, qup1_se4, NA, NA, NA, NA, NA, NA, NA, 0xE0010, 13),
	[88] = PINGROUP(88, qup1_se5, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[89] = PINGROUP(89, qup1_se5, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[90] = PINGROUP(90, qup1_se5, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[91] = PINGROUP(91, qup1_se5, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0010, 14),
	[92] = PINGROUP(92, qup1_se6, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[93] = PINGROUP(93, qup1_se6, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[94] = PINGROUP(94, qup0_se0, ibi_i3c, NA, NA, NA, NA, NA, NA, NA, 0xE0000, 3),
	[95] = PINGROUP(95, qup0_se0, ibi_i3c, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[96] = PINGROUP(96, qup0_se0, NA, phase_flag15, NA, NA, NA, NA, NA, NA, 0, -1),
	[97] = PINGROUP(97, qup0_se0, NA, phase_flag14, NA, NA, NA, NA, NA, NA, 0xE0000, 4),
	[98] = PINGROUP(98, qup0_se1, ibi_i3c, NA, NA, NA, NA, NA, NA, NA, 0xE0000, 5),
	[99] = PINGROUP(99, qup0_se1, ibi_i3c, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[100] = PINGROUP(100, qup0_se1, pwm_0, NA, phase_flag13, NA, NA, NA,
			 NA, NA, 0xE0000, 6),
	[101] = PINGROUP(101, qup0_se1, pwm_1, NA, phase_flag12, NA, NA, NA,
			 NA, NA, 0xE0000, 7),
	[102] = PINGROUP(102, tsense_pwm1, tsense_pwm2, NA, NA, NA, NA, NA, NA,
			 NA, 0, -1),
	[103] = PINGROUP(103, qup1_se6, dp0_hot, NA, NA, NA, NA, NA, NA, NA, 0xE0010, 15),
	[104] = PINGROUP(104, qup1_se6, edp0_hot, NA, NA, NA, NA, NA, NA, NA, 0xE0014, 0),
	[105] = PINGROUP(105, qup1_se6, edp1_hot, NA, NA, NA, NA, NA, NA, NA, 0xE0014, 1),
	[106] = PINGROUP(106, qup1_se6, edp0_lcd, NA, NA, NA, NA, NA, NA, NA, 0xE0014, 2),
	[107] = PINGROUP(107, gcc_gp9, edp1_dpu1, edp1_dpu0, NA, NA, NA, NA,
			 NA, NA, 0, -1),
	[108] = PINGROUP(108, gcc_gp10, pwm_11, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[109] = PINGROUP(109, gcc_gp11, pwm_12, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[110] = PINGROUP(110, mdp0_vsync0, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0014, 3),
	[111] = PINGROUP(111, mdp0_vsync1, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0014, 4),
	[112] = PINGROUP(112, mdp1_vsync0, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0014, 5),
	[113] = PINGROUP(113, mdp1_vsync1, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0000, 8),
	[114] = PINGROUP(114, pwm_16, qdss_cti, NA, NA, NA, NA, NA, NA, NA, 0xE000C, 13),
	[115] = PINGROUP(115, qdss_cti, NA, NA, NA, NA, NA, NA, NA, NA, 0xE000C, 14),
	[116] = PINGROUP(116, ext_mclk1, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[117] = PINGROUP(117, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[118] = PINGROUP(118, pcie0_clk, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0014, 6),
	[119] = PINGROUP(119, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0014, 7),
	[120] = PINGROUP(120, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[121] = PINGROUP(121, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[122] = PINGROUP(122, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0014, 8),
	[123] = PINGROUP(123, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0014, 9),
	[124] = PINGROUP(124, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[125] = PINGROUP(125, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0014, 10),
	[126] = PINGROUP(126, qup0_se5, NA, vsense_trigger, atest_usb0,
			 ddr_pxi0, NA, NA, NA, NA, 0, -1),
	[127] = PINGROUP(127, qup0_se5, NA, atest_usb00, ddr_pxi0, NA, NA, NA,
			 NA, NA, 0, -1),
	[128] = PINGROUP(128, qup0_se5, cmu_rng3, NA, atest_usb01, ddr_pxi1,
			 NA, NA, NA, NA, 0, -1),
	[129] = PINGROUP(129, qup0_se5, cmu_rng2, NA, atest_usb02, ddr_pxi1,
			 NA, NA, NA, NA, 0xE0000, 9),
	[130] = PINGROUP(130, qup0_se4, cmu_rng1, NA, atest_usb03, ddr_pxi2,
			 NA, NA, NA, NA, 0, -1),
	[131] = PINGROUP(131, qup0_se4, cmu_rng0, NA, NA, ddr_pxi2, NA, NA, NA,
			 NA, 0, -1),
	[132] = PINGROUP(132, qup0_se4, NA, NA, ddr_pxi3, NA, NA, NA, NA, NA, 0, -1),
	[133] = PINGROUP(133, qup0_se4, dbg_out, ddr_pxi3, NA, NA, NA, NA, NA,
			 NA, 0xE0000, 10),
	[134] = PINGROUP(134, qup0_se3, qdss_gpio10, NA, NA, NA, NA, NA, NA,
			 NA, 0, -1),
	[135] = PINGROUP(135, qup0_se3, qdss_gpio15, NA, NA, NA, NA, NA, NA,
			 NA, 0, -1),
	[136] = PINGROUP(136, qup0_se3, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[137] = PINGROUP(137, qup0_se3, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0000, 11),
	[138] = PINGROUP(138, gcc_gp1, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[139] = PINGROUP(139, gcc_gp2, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0000, 12),
	[140] = PINGROUP(140, gcc_gp3, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[141] = PINGROUP(141, usb2phy_ac, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[142] = PINGROUP(142, pcie1_clk, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0014, 11),
	[143] = PINGROUP(143, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0014, 12),
	[144] = PINGROUP(144, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[145] = PINGROUP(145, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0014, 13),
	[146] = PINGROUP(146, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0014, 14),
	[147] = PINGROUP(147, edp0_hot, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0014, 15),
	[148] = PINGROUP(148, edp1_hot, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0018, 0),
	[149] = PINGROUP(149, pwm_17, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[150] = PINGROUP(150, qup0_se2, mdp0_vsync0, NA, NA, NA, NA, NA, NA,
			 NA, 0xE0000, 13),
	[151] = PINGROUP(151, qup0_se2, mdp0_vsync1, NA, NA, NA, NA, NA, NA,
			 NA, 0xE0000, 14),
	[152] = PINGROUP(152, qup0_se2, mdp1_vsync0, NA, NA, NA, NA, NA, NA,
			 NA, 0xE0000, 15),
	[153] = PINGROUP(153, qup0_se2, mdp1_vsync1, NA, NA, NA, NA, NA, NA,
			 NA, 0xE0004, 0),
	[154] = PINGROUP(154, i2s2_data1, sdcc5_vdd2, NA, qdss_cti, NA, NA, NA,
			 NA, NA, 0xE0004, 1),
	[155] = PINGROUP(155, i2s2_data0, sd_write, qdss_cti, NA, NA, NA, NA,
			 NA, NA, 0xE0004, 2),
	[156] = PINGROUP(156, i2s2_sck, qup0_se6, NA, qdss_cti, NA, NA, NA, NA,
			 NA, 0, -1),
	[157] = PINGROUP(157, i2s2_ws, qup0_se6, pll_bist, qdss_cti, NA, NA,
			 NA, NA, NA, 0xE0004, 3),
	[158] = PINGROUP(158, qup0_se6, jitter_bist, NA, NA, NA, NA, NA, NA,
			 NA, 0, -1),
	[159] = PINGROUP(159, qup0_se6, pll_clk, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[160] = PINGROUP(160, usb0_phy, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[161] = PINGROUP(161, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[162] = PINGROUP(162, qup0_se4, ext_mclk0, audio_ref, NA, NA, NA, NA,
			 NA, NA, 0xE0004, 4),
	[163] = PINGROUP(163, qup0_se4, i2s0_data0, tgu_ch0, NA, NA, NA, NA,
			 NA, NA, 0xE0004, 5),
	[164] = PINGROUP(164, qup0_se4, i2s0_sck, tgu_ch1, NA, NA, NA, NA, NA,
			 NA, 0xE0004, 6),
	[165] = PINGROUP(165, qup0_se4, i2s0_data1, tgu_ch2, NA, NA, NA, NA,
			 NA, NA, 0xE0004, 7),
	[166] = PINGROUP(166, qup0_se4, i2s0_ws, tgu_ch3, NA, NA, NA, NA, NA,
			 NA, 0xE0004, 8),
	[167] = PINGROUP(167, qup0_se4, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0004, 9),
	[168] = PINGROUP(168, qup0_se4, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0004, 10),
	[169] = PINGROUP(169, pwm_18, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0004, 11),
	[170] = PINGROUP(170, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0004, 12),
	[171] = PINGROUP(171, pcie2_clk, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0004, 13),
	[172] = PINGROUP(172, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0004, 14),
	[173] = PINGROUP(173, pwm_19, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[174] = PINGROUP(174, tb_trig, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[175] = PINGROUP(175, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0004, 15),
	[176] = PINGROUP(176, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[177] = PINGROUP(177, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[178] = PINGROUP(178, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[179] = PINGROUP(179, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[180] = PINGROUP(180, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[181] = PINGROUP(181, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[182] = PINGROUP(182, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[183] = PINGROUP(183, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[184] = PINGROUP(184, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[185] = PINGROUP(185, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[186] = PINGROUP(186, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0008, 0),
	[187] = PINGROUP(187, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[188] = PINGROUP(188, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[189] = PINGROUP(189, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0008, 1),
	[190] = PINGROUP(190, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[191] = PINGROUP(191, qdss_gpio2, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[192] = PINGROUP(192, qdss_gpio3, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[193] = PINGROUP(193, qdss_gpio4, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[194] = PINGROUP(194, qdss_gpio5, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0008, 2),
	[195] = PINGROUP(195, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[196] = PINGROUP(196, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0008, 3),
	[197] = PINGROUP(197, qdss_gpio6, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[198] = PINGROUP(198, qdss_gpio7, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[199] = PINGROUP(199, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0008, 4),
	[200] = PINGROUP(200, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[201] = PINGROUP(201, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0008, 5),
	[202] = PINGROUP(202, qdss_gpio, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[203] = PINGROUP(203, qdss_gpio, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[204] = PINGROUP(204, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[205] = PINGROUP(205, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0008, 6),
	[206] = PINGROUP(206, qdss_gpio8, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[207] = PINGROUP(207, qdss_gpio9, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0008, 7),
	[208] = PINGROUP(208, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0008, 8),
	[209] = PINGROUP(209, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[210] = PINGROUP(210, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0008, 9),
	[211] = PINGROUP(211, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[212] = PINGROUP(212, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0008, 10),
	[213] = PINGROUP(213, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[214] = PINGROUP(214, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[215] = PINGROUP(215, qdss_gpio11, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0008, 11),
	[216] = PINGROUP(216, qdss_gpio12, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[217] = PINGROUP(217, qdss_gpio13, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[218] = PINGROUP(218, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0008, 12),
	[219] = PINGROUP(219, qdss_gpio14, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0008, 13),
	[220] = PINGROUP(220, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[221] = PINGROUP(221, qdss_gpio0, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0008, 14),
	[222] = PINGROUP(222, NA, NA, NA, NA, NA, NA, NA, NA, NA, 0, -1),
	[223] = PINGROUP(223, qdss_gpio1, NA, NA, NA, NA, NA, NA, NA, NA, 0xE0008, 15),
	[224] = UFS_RESET(ufs_reset, 0x1ee000),
	[225] = SDC_QDSD_PINGROUP(sdc2_clk, 0x1e4000, 14, 6),
	[226] = SDC_QDSD_PINGROUP(sdc2_cmd, 0x1e4000, 11, 3),
	[227] = SDC_QDSD_PINGROUP(sdc2_data, 0x1e4000, 9, 0),
};
static struct pinctrl_qup anorak_qup_regs[] = {
};
