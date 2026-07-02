#include "BenchmarkSettings.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

EBenchScenario UBenchmarkSettings::ResolveScenario()
{
	// Override por linha de comando tem prioridade.
	FString Cli;
	if (FParse::Value(FCommandLine::Get(), TEXT("bench="), Cli))
	{
		const FString Lower = Cli.ToLower();
		if (Lower == TEXT("rain") || Lower == TEXT("chuva"))
		{
			return EBenchScenario::Rain;
		}
		if (Lower == TEXT("tower") || Lower == TEXT("torre"))
		{
			return EBenchScenario::Tower;
		}
	}
	return GetDefault<UBenchmarkSettings>()->Scenario;
}
