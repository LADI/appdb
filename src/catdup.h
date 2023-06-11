/* -*- Mode: C ; c-basic-offset: 2 -*- */
/*
 * appdb - Application database via .desktop files
 *
 * Copyright (C) 2009-2023 Nedko Arnaudov
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 *********************************************************
 * This file contains prototype of the catdup() function *
 *********************************************************/

#ifndef CATDUP_H__D42302F1_4D96_4EE4_AC09_E97ED5748277__INCLUDED
#define CATDUP_H__D42302F1_4D96_4EE4_AC09_E97ED5748277__INCLUDED

char * catdup(const char * s1, const char * s2);
char * catdup3(const char * s1, const char * s2, const char * s3);
char * catdup4(const char * s1, const char * s2, const char * s3, const char * s4);
char * catdupv(const char * s1, const char * s2, ...);
char * catdup_array(const char ** array, const char * delimiter);

#endif /* #ifndef CATDUP_H__D42302F1_4D96_4EE4_AC09_E97ED5748277__INCLUDED */
