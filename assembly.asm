; Assembly functions for audio processing
; By Santiago Jimenez

.386 
.model flat,C 

; Function prototypes
calculateAverageVolume PROTO, samples:PTR REAL4, blockSize:SDWORD
findReferenceVolume PROTO, samples:PTR REAL4, blockSize:SDWORD
computeAmplificationFactors PROTO, blockAveragesVolumes:PTR REAL4, referenceVolume:REAL4, amplificationFactors:PTR REAL4, numBlocks:SDWORD
applyAmplificationFactor PROTO, samples:PTR REAL4, amplificationFactor:REAL4, blockSize:SDWORD

.data
; Variables
sum REAL4 0.0
max REAL4 0.0

; Constants
negativeHundred REAL4 -100.0
pointThree REAL4 0.3


.code 

; float calculateAverageVolume(float* samples, int blockSize)

calculateAverageVolume PROC USES eax ebx ecx esi edi,
  samples:PTR REAL4, blockSize:SDWORD

	; Initialize variables
	mov ecx, blockSize
	mov esi, samples
	mov edi, 0

	; Initialize FPU
	finit

	; Initialize sum to zero
	fldz
	fstp sum

	L1:
		;sum += fabs(samples[i]);

		; Load -100.0 into the FPU to check if the end of the samples has been reached
		fld negativeHundred

		; Load sum into the FPU
		fld sum

		; Load samples[i] into the FPU
		fld DWORD PTR [esi + edi * 4]

		; Check if samples[i] is equal to -100.0, if so, jump to special case
		fcomi st(0), st(2)
		je endSamples

		; Calculate fabs(samples[i])
		fabs

		; Add fabs(samples[i]) to sum
		faddp st(1), st(0)
		fstp sum
		jmp notEnd

		; Special case: if (samples[i] == -100.0), fill the rest of the block with 0.3
		endSamples:

		; for (int j = 0; j < blockSize; j++)
			; samples[j] = 0.3;

		mov ebx, blockSize
		mov eax, 0

		; Fill the rest of the block with 0.3
		L1_inner:
			fld pointThree
			fstp DWORD PTR [esi + eax * 4]
			inc eax
			cmp eax, ebx
			jl L1_inner

		; Continue loop
		notEnd:

		;i++;
		inc edi

		;clear stack
		fstp st(0)

		loop L1

	;return sum / blockSize;
	fld sum
	fild blockSize
	fdivp
	
Exit:
	ret
calculateAverageVolume ENDP


; float findReferenceVolume(float* samples, int blockSize)

findReferenceVolume PROC USES eax ecx esi edi,
  samples:PTR REAL4, blockSize:SDWORD

	; Initialize variables
    mov ecx, blockSize
    mov esi, samples
    mov edi, 0
	mov eax, 0

	; Initialize FPU
	finit

	; Initialize max to zero
	fldz
	fstp max
	dec ecx

	; Loop through the samples to find the maximum value
	; for (int i = 0; i < blockSize; i++)
	L2:
		; if (fabs(samples[i]) > max)
		fld max
		fld DWORD PTR [esi + edi * 4]
		fabs

		; Check if fabs(samples[i]) > max
		fcomi st(0), st(1)
		jb NotGreater

		; max = fabs(samples[i])
		fstp max

		;i++;
		inc edi

		;clear stack
		fstp st(0)

		dec ecx
		cmp eax, ecx
		jle L2

	; Return max
	Exit2:
	fld max
	ret

	; If fabs(samples[i]) > max is false, clear the stack and continue loop
	NotGreater:

		inc edi
		cmp eax, ecx
		jge Exit2
		dec ecx

		; clear stack
		fstp st(0)
		fstp st(1)
		jmp L2


findReferenceVolume ENDP

; void computeAmplificationFactors(float* blockAveragesVolumes, float referenceVolume, float* amplificationFactors, int numBlocks)

computeAmplificationFactors PROC USES eax ecx esi edi,
  blockAveragesVolumes:PTR REAL4, referenceVolume:REAL4, amplificationFactors:PTR REAL4, numBlocks:SDWORD

	; Initialize variables
    mov ecx, numBlocks
    mov esi,  amplificationFactors
	mov eax, blockAveragesVolumes
    mov edi, 0

	; Initialize FPU
    finit

	; Loop through the blockAveragesVolumes to calculate amplificationFactors
	L3:
		; Load referenceVolume from memory
		fld ReferenceVolume

		; Load blockAveragesVolumes[i] from memory
		fld DWORD PTR [eax + edi * 4]

		; Calculate amplificationFactors[i] = referenceVolume / blockAveragesVolumes[i]
		fdivp st(1), st(0)

		; Store the result in amplificationFactors[i]
		fstp DWORD PTR [esi + edi * 4]

		; Increment the index (i++)
		inc edi

		; Check loop condition (numBlocks--)
		loop L3   ; Jump to L3 if ecx is not zero

	; Return
	Exit:
		ret

computeAmplificationFactors ENDP

; void applyAmplificationFactor(float* samples, float amplificationFactor, int blockSize)

applyAmplificationFactor PROC USES eax ecx esi edi,
  samples:PTR REAL4, amplificationFactor:REAL4, blockSize:SDWORD

	; Initialize variables
    mov ecx, blockSize
    mov esi, samples
    mov edi, 0
	mov eax, 0

	; Initialize FPU
    finit

	; Initialize to zero
	fldz

	; Loop through the samples to apply the amplificationFactor
	L4:
		; Check if i is out of bounds
		cmp edi, blockSize
		jae Exit

		; samples[i] *= amplificationFactor;
		fld DWORD PTR [esi + edi * 4]
		fld amplificationFactor
		fmulp st(1), st(0)
		fstp DWORD PTR [esi + edi * 4]

		;i++;
		inc edi

		loop L4

	; Return
	Exit:
		ret

applyAmplificationFactor ENDP

END
