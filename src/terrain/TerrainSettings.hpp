#pragma once

#include <FastNoiseLite.h>
#include <map>
#include <print>

namespace Terrain {
	struct NoiseSettings {
		// General settings
		int seed = 1337; // The seed to use for noise generation
		float frequency = 0.01f; // Noise frequency
		FastNoiseLite::NoiseType noise_type = FastNoiseLite::NoiseType_Perlin; // The noise type, perlin by default
		float noise_exp = 1.0f; // Power exponent to use on noise (not too useful but fun)

		// Settings for fractal types
		FastNoiseLite::FractalType fractal_type = FastNoiseLite::FractalType_None; // Fractal type to use
		int fractal_octaves = 3; // Amount of layers to use for fractal noise
		float fractal_lacunarity = 2.0f; // Frequency multiplier between each octave for fractal noise
		float fractal_gain = 0.5f; // Relative strength of noise in each fractal layer compared to last
		float fractal_weighted_strength = 0.0f; // Octave weighting for fractal types;
		float fractal_pingpong_strength = 2.0f; // Strength for ping pong fractal type (not needed for other stuff)

		// Settings for the cellular noise type
		// Distance function for calculating cell for a given point
		FastNoiseLite::CellularDistanceFunction cellular_dist_function = FastNoiseLite::CellularDistanceFunction_EuclideanSq;
		FastNoiseLite::CellularReturnType cellular_return_type = FastNoiseLite::CellularReturnType_Distance; // Value that cellular function returns
		float cellular_jitter = 1.0f; // Maximum distance a cellular point can move from grid pos

		// Domain warp settings
		bool use_domain_warp = false; // Whether or not to use domain warp
		FastNoiseLite::DomainWarpType domain_warp_type = FastNoiseLite::DomainWarpType_OpenSimplex2; // domain warp algorithm
		float domain_warp_amp = 1.0f; // Max warp distance from original pos
		int domain_seed = 1337;
		float domain_frequency = 0.005f;
		// Domain Fractal settings
		FastNoiseLite::FractalType domain_warp_fractal_type = FastNoiseLite::FractalType_None;
		int domain_fractal_octaves = 3;
		float domain_fractal_lacunarity = 2.0f;
		float domain_fractal_gain = 0.5f;

		// Print out settings in struct form to stdout
		// TODO - add domain warp stuff
		void printSettings() const {
			static const std::map<FastNoiseLite::NoiseType, const char *> NOISE_TYPES = {
				{FastNoiseLite::NoiseType_OpenSimplex2, "FastNoiseLite::NoiseType_OpenSimplex2"},
				{FastNoiseLite::NoiseType_OpenSimplex2S, "FastNoiseLite::NoiseType_OpenSimplex2S"},
				{FastNoiseLite::NoiseType_Cellular, "FastNoiseLite::NoiseType_Cellular"},
				{FastNoiseLite::NoiseType_Perlin, "FastNoiseLite::NoiseType_Perlin"},
				{FastNoiseLite::NoiseType_ValueCubic, "FastNoiseLite::NoiseType_ValueCubic"},
				{FastNoiseLite::NoiseType_Value, "FastNoiseLite::NoiseType_Value"}
			};

			static const std::map<FastNoiseLite::FractalType, const char *> FRACTAL_TYPES = {
				{FastNoiseLite::FractalType_None, "FastNoiseLite::FractalType_None"},
				{FastNoiseLite::FractalType_FBm, "FastNoiseLite::FractalType_FBm"},
				{FastNoiseLite::FractalType_Ridged, "FastNoiseLite::FractalType_Ridged"},
				{FastNoiseLite::FractalType_PingPong, "FastNoiseLite::FractalType_PingPong"},
				{FastNoiseLite::FractalType_DomainWarpProgressive, "FastNoiseLite::FractalType_DomainWarpProgressive"},
				{FastNoiseLite::FractalType_DomainWarpIndependent, "FastNoiseLite::FractalType_DomainWarpIndependent"}
			};

			// Map for Cellular Distance Function
			static const std::map<FastNoiseLite::CellularDistanceFunction, const char *> CELLULAR_DISTANCE_FUNCTIONS = {
				{
					FastNoiseLite::CellularDistanceFunction_Euclidean,
					"FastNoiseLite::CellularDistanceFunction_Euclidean"
				},
				{
					FastNoiseLite::CellularDistanceFunction_EuclideanSq,
					"FastNoiseLite::CellularDistanceFunction_EuclideanSq"
				},
				{
					FastNoiseLite::CellularDistanceFunction_Manhattan,
					"FastNoiseLite::CellularDistanceFunction_Manhattan"
				},
				{FastNoiseLite::CellularDistanceFunction_Hybrid, "FastNoiseLite::CellularDistanceFunction_Hybrid"}
			};

			// Map for Cellular Return Type
			static const std::map<FastNoiseLite::CellularReturnType, const char *> CELLULAR_RETURN_TYPES = {
				{FastNoiseLite::CellularReturnType_CellValue, "FastNoiseLite::CellularReturnType_CellValue"},
				{FastNoiseLite::CellularReturnType_Distance, "FastNoiseLite::CellularReturnType_Distance"},
				{FastNoiseLite::CellularReturnType_Distance2, "FastNoiseLite::CellularReturnType_Distance2"},
				{FastNoiseLite::CellularReturnType_Distance2Add, "FastNoiseLite::CellularReturnType_Distance2Add"},
				{FastNoiseLite::CellularReturnType_Distance2Sub, "FastNoiseLite::CellularReturnType_Distance2Sub"},
				{FastNoiseLite::CellularReturnType_Distance2Mul, "FastNoiseLite::CellularReturnType_Distance2Mul"},
				{FastNoiseLite::CellularReturnType_Distance2Div, "FastNoiseLite::CellularReturnType_Distance2Div"}
			};

			static const std::map<FastNoiseLite::DomainWarpType, const char*> DOMAIN_WARP_TYPES = {
				{FastNoiseLite::DomainWarpType_OpenSimplex2, "FastNoiseLite::DomainWarpType_OpenSimplex2"},
				{FastNoiseLite::DomainWarpType_OpenSimplex2Reduced, "FastNoiseLite::DomainWarpType_OpenSimplex2Reduced"},
				{FastNoiseLite::DomainWarpType_BasicGrid, "FastNoiseLite::DomainWarpType_BasicGrid"}
			};

			const char *fractal_type_str = FRACTAL_TYPES.at(fractal_type);
			const char *noise_type_str = NOISE_TYPES.at(noise_type);
			const char *cellular_dist_str = CELLULAR_DISTANCE_FUNCTIONS.at(cellular_dist_function);
			const char *cellular_return_str = CELLULAR_RETURN_TYPES.at(cellular_return_type);
			const char *domain_warp_type_str = DOMAIN_WARP_TYPES.at(domain_warp_type);

			std::printf("NoiseSettings{\n");
			// General settings
			std::printf("\t.seed = %d,\n", seed);
			std::printf("\t.frequency = %gf,\n", frequency);
			std::printf("\t.noise_type = %s,\n", noise_type_str);
			std::printf("\t.noise_exp = %gf,\n", noise_exp);

			// Settings for fractal types
			std::printf("\n"); // Add newline for separation
			std::printf("\t.fractal_type = %s,\n", fractal_type_str);
			std::printf("\t.fractal_octaves = %d,\n", fractal_octaves);
			std::printf("\t.fractal_lacunarity = %gf,\n", fractal_lacunarity);
			std::printf("\t.fractal_gain = %gf,\n", fractal_gain);
			std::printf("\t.fractal_weighted_strength = %gf,\n", fractal_weighted_strength);
			std::printf("\t.fractal_pingpong_strength = %gf,\n", fractal_pingpong_strength);

			// Settings for the cellular noise type
			std::printf("\n"); // Add a newline for separation
			std::printf("\t.cellular_dist_function = %s,\n", cellular_dist_str);
			std::printf("\t.cellular_return_type = %s,\n", cellular_return_str);
			std::printf("\t.cellular_jitter = %gf,\n", cellular_jitter); // Last one shouldn't have a trailing comma

			// Domain warp
			std::printf("\t.domain_warp_type = %s,\n", domain_warp_type_str);
			std::printf("\t.domain_warp_amp = %gf,\n", domain_warp_amp);
			std::printf("\t.domain_seed = %d,\n", domain_seed);
			std::printf("\t.domain_frequency = %gf,\n", domain_frequency);

			// Print the closing brace
			std::printf("};\n");
		}
	};

	namespace Presets {
		static constexpr NoiseSettings testSettings1 = NoiseSettings{
			.seed = 1399,
			.frequency = 0.005f,
			.noise_type = FastNoiseLite::NoiseType_Perlin,
			.noise_exp = 1.0f,

			.fractal_type = FastNoiseLite::FractalType_FBm,
			.fractal_octaves = 4,
			.fractal_lacunarity = 1.74f,
			.fractal_gain = 0.56f,
			.fractal_weighted_strength = 0.0f,
			.fractal_pingpong_strength = 2.0f,

			.cellular_dist_function = FastNoiseLite::CellularDistanceFunction_EuclideanSq,
			.cellular_return_type = FastNoiseLite::CellularReturnType_Distance,
			.cellular_jitter = 1.0f
		};

		static constexpr NoiseSettings cellularHills = NoiseSettings{
			.seed = 1337,
			.frequency = 0.016f,
			.noise_type = FastNoiseLite::NoiseType_Cellular,
			.noise_exp = 1.0f,

			.fractal_type = FastNoiseLite::FractalType_None,
			.fractal_octaves = 3,
			.fractal_lacunarity = 2.0f,
			.fractal_gain = 0.5f,
			.fractal_weighted_strength = 0.0f,
			.fractal_pingpong_strength = 2.0,

			.cellular_dist_function = FastNoiseLite::CellularDistanceFunction_Manhattan,
			.cellular_return_type = FastNoiseLite::CellularReturnType_CellValue,
			.cellular_jitter = 1.0f,
			.domain_warp_type = FastNoiseLite::DomainWarpType_OpenSimplex2,
			.domain_warp_amp = 1.0f,
			.domain_seed = 1337,
			.domain_frequency = 0.005f,
		};

		static constexpr NoiseSettings flatHills = NoiseSettings{
			.seed = 1337,
			.frequency = 0.026f,
			.noise_type = FastNoiseLite::NoiseType_Perlin,
			.noise_exp = 1.0f,

			.fractal_type = FastNoiseLite::FractalType_Ridged,
			.fractal_octaves = 3,
			.fractal_lacunarity = 0.71f,
			.fractal_gain = 1.51f,
			.fractal_weighted_strength = 0.0f,
			.fractal_pingpong_strength = 2.0f,

			.cellular_dist_function = FastNoiseLite::CellularDistanceFunction_Manhattan,
			.cellular_return_type = FastNoiseLite::CellularReturnType_CellValue,
			.cellular_jitter = 1.0f,
			.domain_warp_type = FastNoiseLite::DomainWarpType_OpenSimplex2,
			.domain_warp_amp = 1.0f,
			.domain_seed = 1337,
			.domain_frequency = 0.005f,
		};

		static constexpr NoiseSettings lotsOCircles = NoiseSettings{
			.seed = 1337,
			.frequency = 0.018f,
			.noise_type = FastNoiseLite::NoiseType_Perlin,
			.noise_exp = 1.0f,

			.fractal_type = FastNoiseLite::FractalType_FBm,
			.fractal_octaves = 3,
			.fractal_lacunarity = 0.39f,
			.fractal_gain = 1.39f,
			.fractal_weighted_strength = 0.2f,
			.fractal_pingpong_strength = 2.0f,

			.cellular_dist_function = FastNoiseLite::CellularDistanceFunction_EuclideanSq,
			.cellular_return_type = FastNoiseLite::CellularReturnType_Distance,
			.cellular_jitter = 1.0f,
			.use_domain_warp = true,
			.domain_warp_type = FastNoiseLite::DomainWarpType_OpenSimplex2Reduced,
			.domain_warp_amp = 123.0f,
			.domain_seed = 1337,
			.domain_frequency = 0.034f,
		};

		// Using a larger amplitude helps
		static constexpr NoiseSettings funnyRidges = NoiseSettings{
			.seed = 1337,
			.frequency = 0.005f,
			.noise_type = FastNoiseLite::NoiseType_Perlin,
			.noise_exp = 1.0f,

			.fractal_type = FastNoiseLite::FractalType_Ridged,
			.fractal_octaves = 5,
			.fractal_lacunarity = 1.47f,
			.fractal_gain = 0.41f,
			.fractal_weighted_strength = 0.43f,
			.fractal_pingpong_strength = 2.0f,

			.cellular_dist_function = FastNoiseLite::CellularDistanceFunction_EuclideanSq,
			.cellular_return_type = FastNoiseLite::CellularReturnType_Distance,
			.cellular_jitter = 1.0f,
			.domain_warp_type = FastNoiseLite::DomainWarpType_OpenSimplex2,
			.domain_warp_amp = 1.0f,
			.domain_seed = 1337,
			.domain_frequency = 0.005f,
		};

		static constexpr NoiseSettings bigCircle = NoiseSettings{
			.seed = 1348,
			.frequency = 0.042f,
			.noise_type = FastNoiseLite::NoiseType_Perlin,
			.noise_exp = 1.0f,

			.fractal_type = FastNoiseLite::FractalType_FBm,
			.fractal_octaves = 3,
			.fractal_lacunarity = 0.62f,
			.fractal_gain = 3.0f,
			.fractal_weighted_strength = 0.14f,
			.fractal_pingpong_strength = 2.0f,

			.cellular_dist_function = FastNoiseLite::CellularDistanceFunction_EuclideanSq,
			.cellular_return_type = FastNoiseLite::CellularReturnType_Distance,
			.cellular_jitter = 1.0f,
			.use_domain_warp = true,
			.domain_warp_type = FastNoiseLite::DomainWarpType_OpenSimplex2Reduced,
			.domain_warp_amp = 2000.0f,
			.domain_seed = 1337,
			.domain_frequency = 0.002f,
		};

		const std::map<const char*, NoiseSettings> NOISE_PRESET_MAP = {
			{"Test Settings 1", testSettings1},
			{"Cellular Hills", cellularHills},
			{"Flat Hills", flatHills},
			{"lots O Circles (Domain Warp)", lotsOCircles},
			{"Funny Ridges", funnyRidges},
			{"Big Circle (Domain Warp)", bigCircle}
		};
	}
}
