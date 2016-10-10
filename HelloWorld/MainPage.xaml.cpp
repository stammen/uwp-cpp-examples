//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

#include <ppltasks.h>
#include <experimental\resumable>
#include <pplawait.h>



using namespace HelloWorld;

using namespace concurrency;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::Storage::Pickers;
using namespace Windows::System::Threading;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Media::Imaging;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

std::wstring_convert<std::codecvt_utf8<wchar_t>> MainPage::s_converter;
std::mutex MainPage::s_mutex;

MainPage::MainPage()
{
    InitializeComponent();
}

void MainPage::Convert_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    // convert std::string to Platform::String
    ConvertString("Hello world");
}

void MainPage::ConvertString(const std::string& s)
{
    greetingOutput->Text = "Do Something: " + StringToPlatformString(s);
}

void MainPage::Dispatcher_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    auto dispatcher = Window::Current->CoreWindow->Dispatcher;
    auto t = create_task([this, dispatcher]() {
        //greetingOutput->Text = "Dispatcher: " + StringToPlatformString("Dispatcher_Click");
        dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new DispatchedHandler([=]()
        {
            greetingOutput->Text = "Dispatcher: " + StringToPlatformString("Dispatcher_Click");
        }));
    });
}

std::string MainPage::PlatformStringToString(Platform::String^ s) 
{
    if (s == nullptr) {
        return std::string("");
    }

    std::lock_guard<std::mutex> lock(s_mutex);
    return s_converter.to_bytes(s->Data());
}

Platform::String^ MainPage::StringToPlatformString(const std::string& s) 
{
    if (s.empty()) {
        return ref new Platform::String();
    }
    std::lock_guard<std::mutex> lock(s_mutex);
    std::wstring converted = s_converter.from_bytes(s);
    return ref new Platform::String(converted.c_str());
}

void MainPage::Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    greetingOutput->Text = "Hello, " + nameInput->Text + "!";
}

void MainPage::SaveFileAsync_Click(Platform::Object^ sender, RoutedEventArgs^ e)
{
    auto savePicker = ref new FileSavePicker();
    savePicker->SuggestedStartLocation = PickerLocationId::PicturesLibrary;

    auto extensions = ref new Platform::Collections::Vector<String^>();
    extensions->Append(".txt");
    savePicker->FileTypeChoices->Insert("Plain Text", extensions);
    savePicker->SuggestedFileName = "HelloWorld";

    auto pickerTask = create_task(savePicker->PickSaveFileAsync());

    create_task(pickerTask).then([this](StorageFile^ file)
    {
        //throw ref new InvalidArgumentException();

        if (file == nullptr)
        {
            saveFile->Text = "Operation canceled.";
            cancel_current_task();
        }
        saveFile->Text = "Saved file: " + file->Path;
        return FileIO::WriteTextAsync(file, L"Hello world!");
    })
    .then([this](task<void> t)
    {
        try {
            t.get();
            // .get() didn't throw, so we succeeded.
        }
        catch (Platform::Exception^ ex) {
            saveFile->Text = "Exception: " + ex->Message;
        }
    });
}

void MainPage::SaveFileAwait_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    SaveFileAwait();
}

task<void> MainPage::SaveFileAwait()
{
    auto savePicker = ref new FileSavePicker();
    savePicker->SuggestedStartLocation = PickerLocationId::PicturesLibrary;

    auto extensions = ref new Platform::Collections::Vector<String^>();
    extensions->Append(".txt");
    savePicker->FileTypeChoices->Insert("Plain Text", extensions);
    savePicker->SuggestedFileName = "HelloWorld";

    auto file = co_await savePicker->PickSaveFileAsync();
    if (file == nullptr)
    {
        saveFile->Text = "Operation cancelled";
        return;
    }
    co_await FileIO::WriteTextAsync(file, L"Hello world!");
    saveFile->Text = "Saved file: " + file->Path;
}

void MainPage::DeleteFile_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    DeleteWithTasksHandleErrors(L"HelloWorld.txt");
}


void MainPage::DeleteWithTasksHandleErrors(Platform::String^ fileName)
{
    using namespace Windows::Storage;
    using namespace concurrency;

    StorageFolder^ documentsFolder = KnownFolders::PicturesLibrary;
    auto getFileTask = create_task(documentsFolder->GetFileAsync(fileName));

    getFileTask.then([](StorageFile^ storageFileSample)
    {
        return storageFileSample->DeleteAsync();
    })
        .then([this](task<void> t)
    {
        try
        {
            t.get();
            // .get() didn't throw, so we succeeded.
            deleteFileText->Text = "File deleted";
        }
        catch (Platform::COMException^ ex)
        {
            //Example output: The system cannot find the specified file.
            deleteFileText->Text = "Exception: " + ex->Message;
        }
    });
}

void MainPage::TestFilePicker(Platform::Object^ sender, RoutedEventArgs^ e)
{
    auto t = create_task(AccessCache::StorageApplicationPermissions::FutureAccessList->GetFolderAsync("PickedFolderToken", AccessCache::AccessCacheOptions::None))
    .then([this](StorageFolder^ folder)
    {
        if (folder == nullptr)
        {
            saveFile->Text = "No default folder";
            cancel_current_task();
        }

        return folder->CreateFileAsync("test1.txt", CreationCollisionOption::ReplaceExisting);
    });
    
    t.then([this](task<StorageFile^> previousTask)
    {
        try
        {
            auto file = previousTask.get();
            if (file != nullptr)
            {
                fileText->Text = "Save File: " + file->Path;
            }
            else
            {
                fileText->Text = "Save File: Cancelled";
            }
        }
        catch (Platform::Exception^ ex)
        {
            fileText->Text = "Exception: " + ex->Message;
        }
    });
}

void MainPage::TestFolderPicker(Platform::Object^ sender, RoutedEventArgs^ e)
{
    auto folderPicker = ref new FolderPicker();
    folderPicker->SuggestedStartLocation = PickerLocationId::Desktop;
    folderPicker->FileTypeFilter->Append(".txt");

    create_task(folderPicker->PickSingleFolderAsync())
        .then([this](StorageFolder^ folder)
    {
        if (folder != nullptr)
        {
            AccessCache::StorageApplicationPermissions::FutureAccessList->AddOrReplace("PickedFolderToken", folder);
            folderText->Text = "Picked folder: " + folder->Name;
        }
        else
        {
            folderText->Text = "Operation cancelled.";
            cancel_current_task();
        }
    })
    .then([this](task<void> previousTask)
    {
        try
        {
            previousTask.get();
        }
        catch (Platform::Exception^ ex)
        {
            folderText->Text = "Exception: " + ex->Message;
        }
    });
}


void MainPage::PickImageAsync_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    auto picker = ref new FileOpenPicker();
    picker->FileTypeFilter->Append(L".png");
    picker->SuggestedStartLocation = PickerLocationId::PicturesLibrary;

    auto getFileTask = create_task(picker->PickSingleFileAsync());
    getFileTask.then([this](StorageFile^ file) 
    {
        if (file == nullptr)
        {
            // user cancelled operation
            cancel_current_task();
        }
        //throw ref new InvalidArgumentException();
        return file->OpenReadAsync();
    })
    .then([this](IRandomAccessStreamWithContentType^ stream)
    {
        auto bitmap = ref new BitmapImage();
        bitmap->SetSource(stream);
        theImage->Source = bitmap;
    })
    .then([this](task<void> previousTask)
    {
        try
        {
            previousTask.get();
        }
        catch (Platform::Exception^ ex)
        {
            folderText->Text = "Exception: " + ex->Message;
        }
    });
}

#if 1
void MainPage::PickImageAwait_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    PickImageAwait();
}

task<void> MainPage::PickImageAwait()
{
    auto picker = ref new FileOpenPicker();
    picker->FileTypeFilter->Append(L".png");
    picker->SuggestedStartLocation = PickerLocationId::PicturesLibrary;

    auto file = co_await picker->PickSingleFileAsync();
    if (nullptr == file)
    {
        return;
    }

    auto stream = co_await file->OpenReadAsync();
    auto bitmap = ref new BitmapImage();
    bitmap->SetSource(stream);
    theImage->Source = bitmap;
}

#else
void MainPage::PickImage_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    auto dispatcher = Window::Current->CoreWindow->Dispatcher;  
    auto t = PickAnImage();
    t.then([this, dispatcher](BitmapImage^ bitmap)
    {
        if (bitmap != nullptr)
        {
            dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new DispatchedHandler([=]()
            {
                theImage->Source = bitmap;
            }));
        }
    });
}
#endif

#if 0
// C# version
private async void OnLoadImageClick(object sender, RoutedEventArgs e)
{
    var picker = new FileOpenPicker();
    picker.FileTypeFilter.Add(".png");
    picker.SuggestedStartLocation = PickerLocationId.PicturesLibrary;

    var file = await picker.PickSingleFileAsync();
    if (file == null)
    {
        return;
    }

    var stream = await file.OpenReadAsync();
    var bitmap = new BitmapImage();
    bitmap.SetSource(stream);
    ImageField.Source = bitmap;
}
#endif




