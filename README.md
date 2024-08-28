# BV2NII (experimental)

## Description
A compiled version of a CLI file converter from BrainVoyager's format to NIfTI format written in C++ (more of a practice project).

| File Format | To NIFTI | From NIFTI |
| ----------- | -------- | ---------- |
| VMR         | YES      | YES        |
| V16         | YES      | YES        |
| FMR         | YES      | YES        |
| VTC         | NO       | NO         |

Note: For FMR conversion, a FMR file is needed and the data will be stored into its cooresponding STC file. 

## Dependencies

BrainVoyager's shared code: https://support.brainvoyager.com/software/421-shared-code

NIfTI-1 C library: https://nifti.nimh.nih.gov/nifti-1/

Argumentum: https://github.com/mmahnic/argumentum

Due to copyright issues, only main files are provided.
