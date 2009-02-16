#include "ocr.h"

/*!
 * \brief loads character models
 * \param filename filename to load from
 * \param model_image_width width of the model
 * \param model_image_height height of the model
 * \param models eigenmodel for each character
 * \param average_model average model
 */
void ocr::LoadCharacterModels(
    std::string filename,
	int model_image_width,
	int model_image_height,
	std::vector<float*> &models,
	float* average_model)
{
    ifstream inf;

    inf.open(filename.c_str(), ios::binary);
    if (!inf.good())
    {
        cout << "File not found: " << filename << endl;
    }
    else
    {
    	int model_image_width2 = 0;
    	int model_image_height2 = 0;
		inf.read( (char*) &model_image_width2, sizeof(int));
		inf.read( (char*) &model_image_height2, sizeof(int));
		assert(model_image_width == model_image_width2);
		assert(model_image_height == model_image_height2);
		inf.read( (char *) average_model, model_image_width * model_image_height * sizeof(float));
		int no_of_models = 0;
		inf.read( (char*) &no_of_models, sizeof(int));
		for (int m = 0; m < no_of_models; m++)
		{
			float* model = new float[model_image_width * model_image_height];
			inf.read( (char *) model, model_image_width * model_image_height * sizeof(float));
			models.push_back(model);
		}
		inf.close();
    }
}

/*!
 * \brief saves character models to file
 * \param filename filename to save as
 * \param model_image_width width of the model
 * \param model_image_height height of the model
 * \param models eigenmodel for each character
 * \param average_model average model
 */
void ocr::SaveCharacterModels(
    std::string filename,
    int model_image_width,
	int model_image_height,
	std::vector<float*> &models,
	float* average_model)
{
    std::FILE *file;

    file = fopen(filename.c_str(), "wb");
    assert(file != NULL);

    fwrite(&model_image_width, sizeof(int), 1, file);
    fwrite(&model_image_height, sizeof(int), 1, file);
    fwrite(average_model, sizeof(float), model_image_width * model_image_height, file);
    int no_of_models = (int)models.size();
    fwrite(&no_of_models, sizeof(int), 1, file);
    for (int m = 0; m < no_of_models; m++)
    {
    	fwrite(models[m], sizeof(float), model_image_width * model_image_height, file);
    }

    fclose(file);
}

std::string ocr::RecognizeCharacters(
    int model_image_width,
	int model_image_height,
	std::vector<unsigned char*> observation,
	std::vector<float*> &models,
	float* average_model)
{
	string result = "";
	for (int i = 0; i < (int)observation.size(); i++)
	{
		float similarity = 0;
	    char c = RecognizeCharacter(
	        model_image_width,
	    	model_image_height,
	    	observation[i],
	    	models,
	    	average_model,
	    	similarity);
	    result += c;
	}
	return(result);
}

/*!
 * \brief recognizes an individual character from the given observation
 * \param model_image_width width of the model
 * \param model_image_height height of the model
 * \param observation observed character image
 * \param models eigenmodels for each character
 * \param average_model average character model
 * \param similarity returned difference between the observation and the best fitting eigenmodel
 * \return recognised character
 */
char ocr::RecognizeCharacter(
    int model_image_width,
	int model_image_height,
	unsigned char* observation,
	std::vector<float*> &models,
	float* average_model,
	float &similarity)
{
	char result = ' ';

	// create an eigen image of the observation
	float* eigen_observation = new float[model_image_width * model_image_height];
	for (int i = (model_image_width * model_image_height)-1; i >= 0; i--)
        eigen_observation[i] = (float)observation[i] - average_model[i];

	// find the most similar eigenmodel
	similarity = 0;
	int winner = -1;
	for (int m = 0; m < (int)models.size(); m++)
	{
        float similarity_value = 0;
    	for (int i = (model_image_width * model_image_height)-1; i >= 0; i--)
            similarity_value += ABS(eigen_observation[i] - models[m][i]);

    	if ((m == 0) || (similarity_value < similarity))
    	{
    		similarity = similarity_value;
    		winner = m;
    	}
	}
	if (winner > -1)
	{
	    if (winner < 26)
	    	result = (char)(winner + 65);
	    else
	    	result = (char)(winner - 26 + 48);
	}

	delete[] eigen_observation;
	return(result);
}

/*!
 * \brief turns average character models into eigenmodels
 * \param model_image_width width of the model
 * \param model_image_height height of the model
 * \param models model for each character, which gets converted into an eigenmodel
 * \param average_model returned average model
 */
void ocr::CreateCharacterEigenModels(
    int model_image_width,
	int model_image_height,
	std::vector<float*> &models,
	float* average_model)
{
	memset(average_model, 0, model_image_width * model_image_height * sizeof(float));

	// calculate the average model
	for (int m = 0; m < (int)models.size(); m++)
	{
		for (int i = (model_image_width * model_image_height)-1; i >= 0; i--)
            average_model[i] += models[m][i];
	}

	for (int i = (model_image_width * model_image_height)-1; i >= 0; i--)
        average_model[i] /= (float)models.size();

	// turn the models into eigenmodels by subtracting the average
	for (int m = 0; m < (int)models.size(); m++)
	{
		for (int i = (model_image_width * model_image_height)-1; i >= 0; i--)
  		    models[m][i] -= average_model[i];
	}
}

/*!
 * \brief creates a set of character models from the given directory
 * \param directory which contains subdirectories for each character
 * \param model_image_width width of the bitmaps
 * \param model_image_height height of the bitmaps
 * \param models returned character models
 */
void ocr::CreateCharacterModels(
    std::string characters_directory,
    int model_image_width,
    int model_image_height,
    std::vector<float*> &models)
{
	// A..Z
	for (int c = 0; c < 26; c++)
	{
		char ch = (char)65 + c;
		std::string directory = characters_directory + "/" + ch;

		float* model = new float[model_image_width * model_image_height];
		CreateCharacterModel(directory, model_image_width, model_image_height, model);
		models.push_back(model);
	}

	// 0..9
	for (int c = 0; c < 10; c++)
	{
		char ch = (char)48 + c;
		std::string directory = characters_directory + "/" + ch;

		float* model = new float[model_image_width * model_image_height];
		CreateCharacterModel(directory, model_image_width, model_image_height, model);
		models.push_back(model);
	}
}

/*!
 * \brief create an average model of a character based upon a number of example bitmaps
 * \param character_directory directory within which images for this character are stored
 * \param model_image_width width of the character bitmaps
 * \param model_image_height height of the character bitmaps
 * \param model returned average character model
 */
void ocr::CreateCharacterModel(
    std::string character_directory,
    int model_image_width,
    int model_image_height,
    float* model)
{
	memset(model, 0, model_image_width * model_image_height * sizeof(float));
	int model_hits = 0;

	std::vector<std::string> filenames;
	GetFilesInDirectory(character_directory, filenames);
	for (int i = 0; i < (int)filenames.size(); i++)
	{
		std::string filename = character_directory + "/" + filenames[i];

        Bitmap *bmp = new Bitmap();
        if (bmp->FromFile(filename))
        {
        	unsigned char* char_img = bmp->Data;

        	for (int i = (model_image_width * model_image_height)-1; i >= 0; i--)
        	    model[i] += char_img[i];
        	model_hits++;
        }
        delete bmp;
	}

	if (model_hits > 0)
	{
		for (int i = (model_image_width * model_image_height)-1; i >= 0; i--)
    	    model[i] /= model_hits;
	}
}

/*!
 * \brief returns a list of all filenames within a directory
 * \param dir directory
 * \param filenames list of filenames
 */
void ocr::GetFilesInDirectory(std::string dir, std::vector<std::string> &filenames)
{
	filenames.clear();

    DIR *dp;
    struct dirent *dirp;
    if ((dp = opendir(dir.c_str())) == NULL)
    {
    	cout << "Error opening " << dir << endl;
    	return;
    }

    while ((dirp = readdir(dp)) != NULL)
    {
    	std::string filename = dirp->d_name;
    	if (filename.size() > 3)
    	{
    		if ((filename.substr(filename.size()-3,3) == "bmp") ||
    		    (filename.substr(filename.size()-3,3) == "BMP"))
    		{
    		    //cout << "filename: " << filename << endl;
                filenames.push_back(filename);
    		}
    	}
    }
    std::sort(filenames.begin(), filenames.end());
    closedir(dp);
}
