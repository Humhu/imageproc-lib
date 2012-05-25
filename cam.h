/**
 * Copyright (c) 2011-2012, Regents of the University of California
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of the University of California, Berkeley nor the names
 *   of its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * Image capture device interface
 *
 * by Humphrey Hu
 *
 * v.1.0
 *
 * Usage:
 *
 *  Initialization:
 *
 *  Begin by initializing the module. This sets up the necessary peripherals,
 *  initializes memory and calibrates itself.. Should initialization fail, the
 *  module will not be able to start, but should not produce errors.
 *
 *  ...
 *  camSetup();
 *  ...
 *
 *  Reading:
 *
 *  Captured data is stored in CamFrame objects. The exposed output buffer gives
 *  CamFrame objects in order of oldest to newest. Once processing is done, the
 *  frames must be returned to the camera module.
 *
 *  Latency-sensitive applications can use the CamRow getter, which returns the
 *  newest row in the back frame. These row objects are direct references to rows
 *  in buffered frames, so they do not need to be returned. However, as a result,
 *  they are also volatile and should be processed quickly. Time-of-capture
 *  timestamps are also saved. to each CamRow, and time-of-completion timestamps
 *  are saved to CamFrames.
 *
 *
 *  unsigned int i, j;
 *  unsigned char pixel;
 *  CamFrame frame;
 *  CamRow row;
 *  ...
 *  if(camHasNewFrame()) {
 *      frame = camGetFrame();
 *      for(i = 0; i < frame->num_rows; i++) {
 *          row = frame->rows[i];
 *          for(j = 0; j < frame->num_cols; j++) {
 *              pixel = row->pixels[j];
 *              // row processing here
 *          }
 *      }
 *  }
 *  camReturnFrame(frame);
 *  ...
 *  if(camHasNewRow()) {
 *      row = camGetRow();
 *  }
 */

#ifndef __CAMERA_H
#define __CAMERA_H

#include "ov7660.h"

// Windowing parameters
#define WINDOW_START_COL        (5)   // Start capture at this col
#define WINDOW_END_COL          (155) // End capture at this col
#define WINDOW_START_ROW        (0)   // Start capture at this row
#define WINDOW_END_ROW          (120) // End capture at this row

#define WINDOW_IMAGE_COLS       (WINDOW_END_COL - WINDOW_START_COL)       
#define WINDOW_IMAGE_ROWS       (WINDOW_END_ROW - WINDOW_START_ROW)

// Downsampling parameters
#define DS_COL                  (3) // Capturing 1/DS_COL pixels
#define DS_ROW                  (3) // Capturing 1/DS_ROW rows
#define DS_FRAME                (1) // Capturing 1/DS_FRAME frames

#define DS_IMAGE_COLS           (WINDOW_IMAGE_COLS/DS_COL)
#define DS_IMAGE_ROWS           (WINDOW_IMAGE_ROWS/DS_ROW)

// ============== Typedefs ====================================================

// These are status codes handed to an irq handler registered to the camera
// module that fire upon certain events.
//
// Events:  CAM_IRQ_ROW_DONE = Row capture completed
//          CAM_IRQ_FRAME_DONE = Frame capture completed
//          CAM_IRQ_ERROR = Some driver error (not yet implemented)
typedef enum {
    CAM_IRQ_ROW_DONE = 0,
    CAM_IRQ_FRAME_DONE,
    CAM_IRQ_ERROR,
} CamIrqCause;



/*  Frame object containing the sequence number,
 *  time upon capture completion, and the pixels.
 */
typedef unsigned char CamRow[DS_IMAGE_COLS];
typedef CamRow RowArray[DS_IMAGE_ROWS];

typedef struct {    
    unsigned long timestamp;
    unsigned int frame_num;
    RowArray pixels;
} CamFrame;
 
typedef struct {
    unsigned char type;
    unsigned char active;
    unsigned int pad;
    unsigned long frame_start;
    unsigned long frame_period;
} CamParamStruct;

// Higher level interrupt handler for camera events
typedef void (*CamIrqHandler)(unsigned int irq_cause);
// Camera capture method that takes a buffer and # of pixels to capture
// TODO: Change getRow def from void* to unsigned char*
typedef void (*CamRowGetter)(void *data, unsigned int size);
// Camera method that waits for the beginning of a new frame
typedef void (*CamFrameWaiter)(void);

// ============== Methods =====================================================

// Set up the camera capture module
void camSetup(CamFrame *frames, unsigned int num_frames);

// Measure the capture timings
void camRunCalib(void);

// TODO: Implement!
// Set camera column subsample mode
//void camSetColSubsample(unsigned char);
// Set camera row subsample mode
//void camSetRowSubsample(unsigned char);
// Sleep the camera device (low power mode)
//void camSleep(void);
// Wake the camera device
//void camWake(void);
// Set camera hardware auto exposure mode
//void camSetAutoExposure(unsigned char);

// Begin asynchronous capture procedure
void camStart(void);

// Retrieve camera parameters
void camGetParams(CamParamStruct *params);

// Set function to be called after various events
void camSetIrqHandler(CamIrqHandler irq);

// Returns the next available full frame object
CamFrame* camGetFrame(void);
// Return a frame to the camera
void camReturnFrame(CamFrame *frame);

// See if camera has new frame
unsigned char camHasNewFrame(void);

// Returns current frame/row numbers
unsigned int camGetFrameNum(void);

#endif // __CAMERA_H
