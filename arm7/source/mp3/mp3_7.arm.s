.text                                                 
.arm                                                  

@ desinterleave an mp3 stereo source                      
@ r0 = interleaved data, r1 = left, r2 = right, r3 = len  

    .global AS_StereoDesinterleave
	.type   AS_StereoDesinterleave STT_FUNC

AS_StereoDesinterleave: 
    stmfd sp!, {r4-r11} 

_loop:                  

    ldmia r0!, {r4-r11} 

    strh r4, [r1], #2
    lsr r4,#16
    strh r4, [r2], #2
    subs r3, #1         
    beq _done           

    strh r5, [r1], #2
    lsr r5,#16
    strh r5, [r2], #2
    subs r3, #1         
    beq _done           

    strh r6, [r1], #2   
    lsr r6,#16
    strh r6, [r2], #2
    subs r3, #1         
    beq _done           
   
    strh r7, [r1], #2
    lsr r7,#16
    strh r7, [r2], #2
    subs r3, #1         
    beq _done           

    strh r8, [r1], #2   
    lsr r8,#16
    strh r8, [r2], #2
    subs r3, #1         
    beq _done           

    strh r9, [r1], #2   
    lsr r9,#16
    strh r9, [r2], #2
    subs r3, #1         
    beq _done           

    strh r10, [r1], #2  
    lsr r10,#16
    strh r10, [r2], #2
    subs r3, #1         
    beq _done           

    strh r11, [r1], #2  
    lsr r11,#16
    strh r11, [r2], #2
    subs r3, #1         
    bne _loop           
_done:                  

    ldmia sp!, {r4-r11} 
    bx lr               
.end
