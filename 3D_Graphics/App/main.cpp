#include <App/App.h>

int CALLBACK WinMain(
	HINSTANCE hInstance,     // �츮�� �ۼ��� ���α׷� �ν��Ͻ��� ���� �ڵ�.
	HINSTANCE hPrevInstance, // ����� ������� ����. ������ NULL.
	LPSTR	  lpCmdLine,     // �����. C���α׷� ������ char** argv�� ������ ���ø����̼� ����.
	int       nCmdShow)      // �����츦 ������ ���� ����. �ִ�/�ּ�ȭ ���� ���� ����.
{
	Application* App = nullptr;

	try
	{
		App = new Application(L"Graphics", 800, 600);
		if (App == nullptr)
		{
			return 0;
		}

		if (App->Initialize())
		{
			App->Run();
		}
	}
	catch (const CustomException& e)
	{
		MessageBoxA(nullptr, e.what(), e.GetType(), MB_OK | MB_ICONEXCLAMATION);
	}
	catch (const std::exception& e)
	{
		MessageBoxA(nullptr, e.what(), "Standard Exception", MB_OK | MB_ICONEXCLAMATION);
	}
	catch (...)
	{
		MessageBoxA(nullptr, "No details available", "Unknown Exception", MB_OK | MB_ICONEXCLAMATION);
	}

	if (App != nullptr)
	{
		App->ShutDown();
		delete App;
		App = nullptr;
	}

	return 0;
}