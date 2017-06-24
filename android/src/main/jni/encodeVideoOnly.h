/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * Author: ramneek
 *
 * Created on 13 June, 2017, 2:52 AM
 */

#ifndef MAKE_GIF_H
#define MAKE_GIF_H

typedef void (*progress)(void *, int);
typedef void (*done)(void *);
typedef void (*error)(void *, char *);

void encodeVideoOnly(const char *infile,
                     const char *outfile,
                     const char *out_codec_str,
                     const char *filter_spec,
                     void * vp,
                     progress p,
                     done d,
                     error e);

#endif

