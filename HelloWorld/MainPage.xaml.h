//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"
#include <codecvt>
#include <string>
#include <mutex>

namespace HelloWorld
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class MainPage sealed
	{
	public:
		MainPage();

	internal:
		void ConvertString(const std::string& s);

	private:
		void Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void PickImage_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void SaveFileAsync_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void SaveFileAwait_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		Concurrency::task<void> SaveFileAwait();
		void TestFolderPicker(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void TestFilePicker(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void PickImageAsync_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void PickImageAwait_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		Concurrency::task<void> PickImageAwait();
		void DeleteFile_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void DeleteWithTasksHandleErrors(Platform::String^ fileName);

		void Convert_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Dispatcher_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		std::string PlatformStringToString(Platform::String^ s);
		Platform::String^ StringToPlatformString(const std::string& s);


		static std::wstring_convert<std::codecvt_utf8<wchar_t>> s_converter;
		static std::mutex s_mutex;
	};
}
