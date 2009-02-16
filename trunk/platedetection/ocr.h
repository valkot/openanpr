#ifndef OCR_H_
#define OCR_H_

#ifndef ABS
#define ABS(a) (((a) < 0) ? -(a) : (a))
#endif

#include <math.h>
#include <string>
#include <cstdlib>
#include <vector>
#include "../common.h"
#include "../utils/Image.h"
#include "../utils/polygon.h"
#include "../shapes/shapes.h"
#include "platedetection.h"
#include "platereader.h"

class ocr {
private:
	static void GetFilesInDirectory(
	    std::string dir,
	    std::vector<std::string> &filenames);
public:
	static void LoadCharacterModels(
	    std::string filename,
		int model_image_width,
		int model_image_height,
		std::vector<float*> &models,
		float* average_model);

	static void SaveCharacterModels(
	    std::string filename,
	    int model_image_width,
		int model_image_height,
		std::vector<float*> &models,
		float* average_model);

	static std::string RecognizeCharacters(
	    int model_image_width,
		int model_image_height,
		std::vector<unsigned char*> observation,
		std::vector<float*> &models,
		float* average_model);

	static char RecognizeCharacter(
	    int model_image_width,
		int model_image_height,
		unsigned char* observation,
		std::vector<float*> &models,
		float* average_model,
		float &similarity);

	static void CreateCharacterEigenModels(
	    int model_image_width,
		int model_image_height,
		std::vector<float*> &models,
		float* average_model);

	static void CreateCharacterModels(
	    std::string characters_directory,
	    int model_image_width,
	    int model_image_height,
	    std::vector<float*> &models);

	static void CreateCharacterModel(
	    std::string character_directory,
	    int model_image_width,
	    int model_image_height,
	    float* model);
};

#endif /* OCR_H_ */
