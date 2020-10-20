/*
 * Copyright (c) 2019 Stephen Ramm
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 */

/*!
 * \file    example1.c
 * \example example1
 * \brief   Use the image creation functions to create a white rectangle on a black background
 * and count the corners in the result using the Fast Corners function.
 * \author  Stephen Ramm <stephen.v.ramm@gmail.com>
 */


/* Steps
1) Create an OpenVX context
2) Create an image that is a white rectangle on a black background
3) Locate the corners in the image, using the Fast Corners algorithm with and without non-maximum suppression
4) Display the results


*/

/*** Non-maximal Suppression ***/

/* Detecting multiple interest points in adjacent locations is another problem. It is solved by using Non-maximum Suppression.

1) Compute a score function, V for all the detected feature points. V is the sum of absolute difference between p and 16 surrounding pixels values.
2) Consider two adjacent keypoints and compute their V values.
3) Discard the one with lower V value.

*/

#include <stdio.h>
#include <stdlib.h>
#include <VX/vx.h>
#include <VX/vxu.h>


void errorCheck(vx_context *context_p, vx_status status, const char *message)
{
    if(status){
        puts("Error !");
        puts(message);
        vxReleaseContext(context_p);
        exit(1);
    }
}


vx_image makeInputImage(vx_context context){

    vx_image image = vxCreateImage(context, 100U, 100U, VX_DF_IMAGE_U8);
    vx_rectangle_t rect = {
        .start_x = 20, .start_y = 40, .end_x = 80, .end_y = 60
    };

    if(VX_SUCCESS == vxGetStatus((vx_reference)image)){
        vx_image roi = vxCreateImageFromROI(image, &rect);
        vx_pixel_value_t pixel_white, pixel_black;
        pixel_white.U8 = 255;
        pixel_black.U8 = 0;

        if(VX_SUCCESS == vxGetStatus((vx_reference)roi) &&
           VX_SUCCESS == vxSetImagePixelValues(image, &pixel_black) &&
           VX_SUCCESS == vxSetImagePixelValues(roi, &pixel_white))
            vxReleaseImage(&roi);
        else
        {
            vxReleaseImage(&image);
        }   
    }
    return image;
}
int main(){

    vx_context context = vxCreateContext();
    
    errorCheck(&context, vxGetStatus((vx_reference)context), 
    "Could not create a vx_content \n");

    vx_image image1 = makeInputImage(context);
    errorCheck(&context, vxGetStatus((vx_reference)image1),
    "Could not create an image \n");

    vx_float32 strength_thresh_value = 128.0;
    vx_scalar strength_thresh = vxCreateScalar(context, VX_TYPE_FLOAT32, & strength_thresh_value);

    // 100 - capacity)
    vx_array corners = vxCreateArray(context, VX_TYPE_KEYPOINT, 100);
    vx_array corners1 = vxCreateArray(context, VX_TYPE_KEYPOINT, 100);

    vx_size num_corners_value = 0;
    vx_scalar num_corners = vxCreateScalar(context, VX_TYPE_SIZE, &num_corners_value);
    vx_scalar num_corners1 = vxCreateScalar(context, VX_TYPE_SIZE, &num_corners_value);
    // struct vx_keypoint_t 
    vx_keypoint_t *kp = calloc(100, sizeof(vx_keypoint_t));

    errorCheck(&context, kp == NULL || 
            vxGetStatus((vx_reference)strength_thresh) || 
            vxGetStatus((vx_reference)corners) ||
            vxGetStatus((vx_reference)num_corners) || 
            vxGetStatus((vx_reference)corners1) ||
            vxGetStatus((vx_reference)num_corners1),
            "Could not create parameters for FastCorners");

    // If true, non-maximum suppression is applied to detected corners before being places in the vx_array of VX_TYPE_KEYPOINT structs.

    errorCheck(&context, vxuFastCorners(context, image1, strength_thresh, vx_true_e, corners, num_corners), "Fast Corners function failed");
    errorCheck(&context, vxuFastCorners(context, image1, strength_thresh, vx_false_e, corners1, num_corners1), "Fast Corners fuction failed");

    // VX_READ_ONLY means that data are copied from the scalar object into the user memory.
    errorCheck(&context, vxCopyScalar(num_corners, &num_corners_value, VX_READ_ONLY, VX_MEMORY_TYPE_HOST), "vxCopyScalar failed");

    printf("Found %zu corners with non-max suppression \n",num_corners_value );
    // 0 - array range start, num_corners_value - array range end
    errorCheck(&context, vxCopyArrayRange(corners, 0, num_corners_value, sizeof(vx_keypoint_t), kp, VX_READ_ONLY, VX_MEMORY_TYPE_HOST), "vxCopyArrayRange failed");
    
    for (int i = 0; i <num_corners_value; i++){
        printf("Entry %3d:x = %d, y = %d\n", i, kp[i].x, kp[i].y);
    
    }
    errorCheck(&context, vxCopyScalar(num_corners1, &num_corners_value, VX_READ_ONLY, VX_MEMORY_TYPE_HOST), "vxCopyScalar failed");
    
    printf("Found %zu corners without non-max suppression \n", num_corners_value);

    errorCheck(&context, vxCopyArrayRange(corners1, 0, num_corners_value, sizeof(vx_keypoint_t), kp, VX_READ_ONLY, VX_MEMORY_TYPE_HOST), "vxCopyArrayRange failed");

    for(int i =0; i<num_corners_value; i++)
    {
        printf("Entry %3d: x = %d, y = %d \n", i , kp[i].x, kp[i].y);
    }
    // free kp
    free(kp);

    // Release the context
    vxReleaseContext(&context);
    return 0;
}


