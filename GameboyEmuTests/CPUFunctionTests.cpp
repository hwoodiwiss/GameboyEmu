#include "CppUnitTest.h"
#include <GameboyEmu/CPU_Functions.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace GameboyEmuTests::CPU
{
	TEST_CLASS(CPUFunctionTests)
	{
	public:

		TEST_METHOD(TestLDByteByte)
		{
			BYTE operand = 107;
			BYTE reg = -1;

			LD(reg, operand);

			Assert::AreEqual(operand, reg);
		}
	};
}
