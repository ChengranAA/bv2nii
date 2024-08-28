#include <iostream>
#include <string>
#include <filesystem>
#include <cstdlib>

#include "argumentum/argparse.h"
#include "volume.h"
#include "myv16.h"
#include "myvmr.h"
#include "myFMR.h"
#include "nifti1.h"
#include "nifti1_io.h"
#include "nifti1_tool.h"

using namespace argumentum;
namespace fs = std::filesystem;

typedef enum
{
    F_VMR,
    F_V16,
    F_VTC,
    F_FMR,
    F_NII
} FILE_TYPE;

typedef struct
{
    std::string file_in;
    FILE_TYPE file_in_type;
    std::string file_out;
    FILE_TYPE file_out_type;
    int compression;
} app_config;

FILE_TYPE get_extension(std::string file)
{
    if (fs::path(file).extension() == ".gz")
    {
        if (fs::path(file).stem().extension() == ".nii") return F_NII;
    }
    else if (fs::path(file).extension() == ".nii") return F_NII;
    else if (fs::path(file).extension() == ".vmr") return F_VMR;
    else if (fs::path(file).extension() == ".v16") return F_V16;
    else if (fs::path(file).extension() == ".fmr") return F_FMR;
    else if (fs::path(file).extension() == ".vtc") return F_VTC;
    else
    {
        assert("File format not supported");
        exit(1);
    }
}

std::uint8_t * data_to_uint8(nifti_image *nim)
{
    std::uint8_t *uint8Data = new std::uint8_t[nim->nvox];

    switch (nim->datatype)
    {

    case NIFTI_TYPE_UINT8:
        std::memcpy(uint8Data, nim->data, nim->nvox * sizeof(std::uint8_t));
        break;

    case NIFTI_TYPE_INT8:
    {
        std::int8_t *data = static_cast<std::int8_t *>(nim->data);
        for (size_t i = 0; i < nim->nvox; ++i)
        {
            uint8Data[i] = static_cast<std::uint8_t>(data[i]);
        }
        break;
    }

    default:
        std::cerr << "Unsupported data type, or error in data size check." << std::endl;
        delete[] uint8Data;
        return nullptr;
    }

    return uint8Data;
}

std::uint16_t* data_to_uint16(nifti_image* nim) {
    // Allocate memory for the new uint16_t array
    std::uint16_t* uint16Data = new std::uint16_t[nim->nvox];

    switch (nim->datatype) {

    case NIFTI_TYPE_UINT8:
    {
        std::uint8_t* data = static_cast<std::uint8_t*>(nim->data);
        for (size_t i = 0; i < nim->nvox; ++i) {
            uint16Data[i] = static_cast<std::uint16_t>(data[i]);
        }
        break;
    }

    case NIFTI_TYPE_INT8:
    {
        std::int8_t* data = static_cast<std::int8_t*>(nim->data);
        for (size_t i = 0; i < nim->nvox; ++i) {
            uint16Data[i] = static_cast<std::uint16_t>(data[i]);
        }
        break;
    }

    case NIFTI_TYPE_UINT16:
        // If it's already uint16_t, we can just copy it
        std::memcpy(uint16Data, nim->data, nim->nvox * sizeof(std::uint16_t));
        break;

    case NIFTI_TYPE_INT16:
    {
        std::int16_t* data = static_cast<std::int16_t*>(nim->data);
        for (size_t i = 0; i < nim->nvox; ++i) {
            uint16Data[i] = static_cast<std::uint16_t>(data[i]);
        }
        break;
    }

    default:
        std::cerr << "Unsupported data type, or error in data size check." << std::endl;
        delete[] uint16Data;
        return nullptr;
    }

    return uint16Data;
}

float* data_to_float(nifti_image *nim)
{
    // Allocate memory for the new float array
    float* floatData = new float[nim->nvox];

    switch (nim->datatype)
    {

    case NIFTI_TYPE_UINT8:
    {
        std::uint8_t* data = static_cast<std::uint8_t*>(nim->data);
        for (size_t i = 0; i < nim->nvox; ++i)
        {
            floatData[i] = static_cast<float>(data[i]);
        }
        break;
    }

    case NIFTI_TYPE_INT8:
    {
        std::int8_t* data = static_cast<std::int8_t*>(nim->data);
        for (size_t i = 0; i < nim->nvox; ++i)
        {
            floatData[i] = static_cast<float>(data[i]);
        }
        break;
    }

    case NIFTI_TYPE_UINT16:
    {
        std::uint16_t* data = static_cast<std::uint16_t*>(nim->data);
        for (size_t i = 0; i < nim->nvox; ++i)
        {
            floatData[i] = static_cast<float>(data[i]);
        }
        break;
    }

    case NIFTI_TYPE_INT16:
    {
        std::int16_t* data = static_cast<std::int16_t*>(nim->data);
        for (size_t i = 0; i < nim->nvox; ++i)
        {
            floatData[i] = static_cast<float>(data[i]);
        }
        break;
    }

    case NIFTI_TYPE_FLOAT32:
        std::memcpy(floatData, nim->data, nim->nvox * sizeof(float));
        break;

    default:
        std::cerr << "Unsupported data type, or error in data size check." << std::endl;
        delete[] floatData;
        return nullptr;
    }

    return floatData;
}

void vmr2nii(app_config current_config)
{
    if (current_config.file_in_type == F_VMR && current_config.file_out_type == F_NII)
    {
        // convert vmr to nii
        VMR new_vmr;
        new_vmr.read_from_file(current_config.file_in);

        nifti_image *nim = nifti_simple_init_nim();
        nim->nx = new_vmr.data.nx();
        nim->ny = new_vmr.data.ny();
        nim->nz = new_vmr.data.nz();

        nim->nt = 1;
        nim->nvox = nim->nx * nim->ny * nim->nz * nim->nt;

        nim->datatype = NIFTI_TYPE_UINT8;
        nim->nbyper = sizeof(std::uint8_t);
        nim->data = calloc(nim->nvox, nim->nbyper);
        nim->data = new_vmr.data.data();

        nifti_set_filenames(nim, current_config.file_out.c_str(), 0, 1);
        nifti_image_write(nim);
    }

    if (current_config.file_in_type == F_NII && current_config.file_out_type == F_VMR)
    {
        // convert nii to vmr
        char *file_name_char = new char[current_config.file_in.size() + 1];
        strcpy(file_name_char, current_config.file_in.c_str());

        nifti_image *nim = nifti_image_read(file_name_char, 1);
        VMR new_vmr;

        std::uint8_t *uint8Data = data_to_uint8(nim);

        Volume tmp = Volume<std::uint8_t>(nim->nx, nim->ny, nim->nx, 0);
        tmp.copyData(uint8Data);
        new_vmr.data = tmp;
        new_vmr.write_to_file(current_config.file_out);
        delete[] uint8Data;
        nifti_image_free(nim);
    }

    return;
}

void v162nii(app_config current_config)
{
    if (current_config.file_in_type == F_V16 && current_config.file_out_type == F_NII)
    {
        // convert vmr to nii
        V16 new_v16;
        new_v16.read_from_file(current_config.file_in);

        nifti_image *nim = nifti_simple_init_nim();
        nim->nx = new_v16.data.nx();
        nim->ny = new_v16.data.ny();
        nim->nz = new_v16.data.nz();

        nim->nt = 1;
        nim->nvox = nim->nx * nim->ny * nim->nz * nim->nt;

        nim->datatype = NIFTI_TYPE_UINT16;
        nim->nbyper = sizeof(std::uint16_t);
        nim->data = calloc(nim->nvox, nim->nbyper);
        nim->data = new_v16.data.data();

        nifti_set_filenames(nim, current_config.file_out.c_str(), 0, 1);
        nifti_image_write(nim);
    }

    if (current_config.file_in_type == F_NII && current_config.file_out_type == F_V16)
    {
        // convert nii to vmr
        char *file_name_char = new char[current_config.file_in.size() + 1];
        strcpy(file_name_char, current_config.file_in.c_str());

        nifti_image *nim = nifti_image_read(file_name_char, 1);
        V16 new_v16;

        std::uint16_t *uint16Data = data_to_uint16(nim);

        Volume tmp = Volume<std::uint16_t>(nim->nx, nim->ny, nim->nx, 0);
        tmp.copyData(uint16Data);
        new_v16.data = tmp;
        new_v16.write_to_file(current_config.file_out);
        delete[] uint16Data;
        nifti_image_free(nim);
    }

    return;
}

void fmr2nii(app_config current_config)
{
    if (current_config.file_in_type == F_FMR && current_config.file_out_type == F_NII)
    {
        struct FMR new_fmr(current_config.file_in);
        nifti_image *nim = nifti_simple_init_nim();

        nim->nx = new_fmr.STCData[0].nx();
        nim->ny = new_fmr.STCData[0].ny();
        nim->nz = new_fmr.STCData[0].nz();
        nim->nt = new_fmr.STCData.size();

        nim->nvox = nim->nx * nim->ny * nim->nz * nim->nt;

        nim->datatype = NIFTI_TYPE_FLOAT32;
        nim->nbyper = sizeof(float);
        nim->data = new float[nim->nvox];

        float *dest_ptr = static_cast<float *>(nim->data);

        for (const auto &volume : new_fmr.STCData)
        {
            std::memcpy(dest_ptr, volume.data(), volume.size() * sizeof(float));
            dest_ptr += volume.size();
        }

        nifti_set_filenames(nim, current_config.file_out.c_str(), 0, 1);
        nifti_image_write(nim);
    }

    if (current_config.file_in_type == F_NII && current_config.file_out_type == F_FMR)
    {
        char *file_name_char = new char[current_config.file_in.size() + 1];
        strcpy(file_name_char, current_config.file_in.c_str());

        nifti_image *nim = nifti_image_read(file_name_char, 1);
        struct FMR new_fmr(current_config.file_out, false);

        // check if out file exists 
        if (!std::filesystem::exists(current_config.file_out)) std::cerr << "FMR file not found" << std::endl;

        float* float32data = data_to_float(nim);
        std::vector<Volume<float>> STCData(nim->nt, Volume<float>(nim->nx, nim->ny, nim->nz));

        for (int t = 0; t < nim->nt; ++t) {
            STCData[t].copyData(float32data + t * STCData[0].size());
        }

        new_fmr.STCData = STCData;
        new_fmr.Save();

        delete[] float32data;
        nifti_image_free(nim);
    }
    return; 
}

app_config cli_config(int argc, char **argv)
{

    app_config current_config;

    auto parser = argument_parser{};
    auto params = parser.params();
    parser.config().program(argv[0]).description("bv2nii");
    params.add_parameter(current_config.file_in, "file_in")
        .required(true)
        .help("Input file name");

    params.add_parameter(current_config.file_out, "-o", "--output")
        .required(true)
        .nargs(1)
        .help("Output file name");

    if (!parser.parse_args(argc, argv, 1))
        exit(1);

    current_config.file_in_type = get_extension(current_config.file_in);
    current_config.file_out_type = get_extension(current_config.file_out);

    return current_config;
}

int main(int argc, char **argv)
{
    app_config current_config = cli_config(argc, argv);
    if (current_config.file_in_type == F_VMR || current_config.file_out_type == F_VMR) vmr2nii(current_config);
    if (current_config.file_in_type == F_FMR || current_config.file_out_type == F_FMR) fmr2nii(current_config);
    if (current_config.file_in_type == F_V16 || current_config.file_out_type == F_V16) v162nii(current_config);
    return 0;
}