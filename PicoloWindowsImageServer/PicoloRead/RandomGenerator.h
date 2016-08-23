#ifndef RANDOM_GENERATOR_H_
#define RANDOM_GENERATOR_H_
#include <random>
#include <memory>//std::shared_ptr
class RandomGenerator{
public:
	typedef int IntegerType;
	typedef std::uniform_int_distribution<IntegerType> IntegerDistribution;
	RandomGenerator(){
		std::random_device r;
		engine.seed(r());
	}

	void initIntDistribution(const IntegerType& minInt, const IntegerType& maxInt){

		intGen.reset(new IntegerDistribution(minInt, maxInt));
	}

	IntegerType randInt(){
		if (intGen){
			return (*intGen)(engine);
		}
	}

protected:
	std::mt19937 engine;
	std::shared_ptr<IntegerDistribution> intGen;
};
#endif