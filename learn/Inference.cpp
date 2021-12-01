#include <torch/script.h> // One-stop header.
#include <cmath>
#include <iostream>
#include <iomanip>
#include <memory>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <tuple>
#include <map>
#include <list>
#include <algorithm>
#include <regex>

using namespace std;
#define kIMAGE_SIZE 112
#define kCHANNELS 3
  map<int, int> map_labels = {
    {0, 10},
    {1, 100},
    {2, 120},
    {3, 130},
    {4, 20},
    {5, 30},
	{6, 40},
    {7, 50},
    {8, 60},
	{9, 70},
	{10, 80}
};
		
// void TableFormat(Table Inference){
	// Inference.format()
	// .font_style({FontStyle::bold})
	// .border_top(" ")
	// .border_bottom(" ")
	// .border_left(" ")
	// .border_right(" ")
	// .corner(" ");
    
	// Inference[0].format()
	// .padding_top(1)
	// .padding_bottom(1)
	// .font_align(FontAlign::center)
	// .font_style({FontStyle::underline})
	// .font_background_color(Color::red);

	// Inference.column(1).format()
	// .font_color(Color::yellow);

	// Inference[0][1].format()
	// .font_background_color(Color::blue)
	// .font_color(Color::white);
	// Inference[0][0].format()
	// .font_background_color(Color::green)
	// .font_color(Color::white);
	// }
bool LoadImage(std::string file_name, cv::Mat &image) {
  image = cv::imread(file_name); 

  if (image.empty() || !image.data) {
    return false;
  }
  cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
//  std::cout << "== image size: " << image.size() << " ==" << std::endl;

  // scale image to fit
  cv::Size scale(kIMAGE_SIZE, kIMAGE_SIZE);
  cv::resize(image, image, scale);
//  std::cout << "== simply resize: " << image.size() << " ==" << std::endl;

  // convert [unsigned int] to [float]
  image.convertTo(image, CV_32FC3, 1.0f / 255.0f);

  return true;
}


int main(int argc, const char *argv[]) {
	//set seed for DL
	torch::manual_seed(42);
	
	//load image & model 
	cv::Mat image;
	if (LoadImage(argv[2], image)){
		 torch::jit::script::Module module = torch::jit::load(argv[1]);


      torch::Tensor input_tensor = torch::from_blob(
          image.data, {1, kIMAGE_SIZE, kIMAGE_SIZE, kCHANNELS});
      input_tensor = input_tensor.permute({0, 3, 1, 2});

	//make inference
	auto output = module.forward({input_tensor}).toTuple()->elements()[0].toTensor();

	auto log_softmaxs = output.log_softmax(1);
	auto softmaxs = torch::exp(log_softmaxs);
	auto proba_top3 = torch::topk(softmaxs, 3);
	
	auto idx = get<1>(proba_top3)[0][0].item<int>();

  	cout<<map_labels[idx]<<endl;
 	}
}
//	Table Inference;

//	Inference.add_row({"Top-k", "Label", "Probability"});

     //get the probability table
//      for (int i = 0; i<3; ++i) 
//      {
//	auto idx = get<1>(proba_top3)[0][i].item<int>();
//	auto label = map_labels[to_string(label_list[idx])];
//	auto proba = get<0>(proba_top3)[0][i].item<float>()*100.0f;
//	
//	Inference.add_row({to_string(i+1), label, to_string(proba)+"%"});
//	
//	}
//	TableFormat(Inference);
//	Inference.print(std::cout);
//	
	// }


