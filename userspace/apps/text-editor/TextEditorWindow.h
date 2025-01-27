#include <libfilepicker/FilePicker.h>
#include <libio/Streams.h>

#include <libwidget/Components.h>
#include <libwidget/Layouts.h>
#include <libwidget/Window.h>
#include <libwidget/dialog/MessageBox.h>
#include <libwidget/model/TextModel.h>

using namespace Widget;

struct TextEditorWindow : public Window
{
private:
    RefPtr<TitleBarComponent> _titlebar;
    RefPtr<TextEditor> _text_editor;
    RefPtr<TextModel> _text_model;
    OwnPtr<Async::Observer<Widget::TextModel>> _text_observer;

    bool _modified = false;
    String _path = "";
    String _default_title = "Text Editor";
    String _title = _default_title;

public:
    TextEditorWindow(String path) : Window(WINDOW_RESIZABLE)
    {
        _titlebar = titlebar(Graphic::Icon::get("text-box"), _title);
        _text_model = TextModel::empty();
        _text_editor = texteditor(_text_model);

        size(Math::Vec2i(700, 500));
        load_document(path);
        _text_observer = _text_model->observe([this](auto &) {
            if (!_modified)
            {
                _modified = true;

                _titlebar->title(generate_title(_path, "*"));

                should_relayout();
            }
        });

        on(EventType::KEYBOARD_KEY_TYPED, [this](Event *event) {
            if (event->keyboard.key == Key::KEYBOARD_KEY_O && event->keyboard.modifiers == KEY_MODIFIER_CTRL)
            {
                on_load();
            }
            else if (event->keyboard.key == Key::KEYBOARD_KEY_S && event->keyboard.modifiers == KEY_MODIFIER_CTRL)
            {
                on_save();
            }
            else if (event->keyboard.key == Key::KEYBOARD_KEY_W && event->keyboard.modifiers == KEY_MODIFIER_CTRL)
            {
                try_hide();
            }
            else
            {
                event->accepted = false;
            }
        });

        on(EventType::WINDOW_CLOSING, [this](Event *event) {
            bool keep = false;
            if (_modified)
            {
                show_warning([&](auto result) {
                    switch (result)
                    {
                    case DialogResult::YES:
                        on_save();
                        break;
                    case DialogResult::NO:
                        break;
                    default:
                        keep = true;
                        break;
                    }
                });
            }

            event->accepted = keep;
        });
    }

    void load_document(String path)
    {
        if (!path.null_or_empty())
        {
            IO::logln("Opening text document from {}", path);
            _text_model = TextModel::open(path);

            _path = path;
            _modified = false;

            _titlebar->title(generate_title(_path));

            _text_editor->update_model(_text_model);
            should_relayout();
        }
    }

    void save_document(String path)
    {
        if (!path.null_or_empty())
        {
            IO::logln("Saving text document to {}", path);
            if (_text_model->save(path).success())
            {
                _path = path;
                _modified = false;

                _titlebar->title(generate_title(path));

                _text_editor->focus();
                should_relayout();
            }
        }
    }

    void on_save()
    {
        auto path_is_null_or_empty = _path.null_or_empty();
        if (_modified && !path_is_null_or_empty)
        {
            save_document(_path);
        }
        else if (path_is_null_or_empty)
        {
            FilePicker::Dialog picker{FilePicker::DIALOG_FLAGS_SAVE};
            if (picker.show() == DialogResult::OK)
            {
                save_document(picker.selected_file().unwrap());
            }
        }
    };

    bool on_load()
    {

        bool show_picker = true;
        if (_modified)
        {
            show_warning([&](auto result) {
                switch (result)
                {
                case DialogResult::YES:
                    on_save();
                    break;
                case DialogResult::NO:
                    break;
                default:
                    show_picker = false;
                    break;
                }
            });
        }

        if (show_picker)
        {
            FilePicker::Dialog picker{};

            if (picker.show() == DialogResult::OK)
            {
                load_document(picker.selected_file().unwrap());
            }

            return true;
        }

        return false;
    }

    void show_warning(Callback<void(DialogResult)> callback)
    {
        auto title = IO::format("Do you want to save document \"{}\"?", _path);

        auto dialog_result = Widget::MessageBox::
            create_and_show(title,
                            "If you don't save, any changes will be lost forever!",
                            Graphic::Icon::get("warning"),
                            DialogButton::YES | DialogButton::NO | DialogButton::CANCEL);
        callback(dialog_result);
    }

    String generate_title(String path, String modifier = "")
    {
        auto str_builder = StringBuilder();
        str_builder.append(_default_title);

        if (!path.null_or_empty())
        {
            str_builder.append(" - ").append(path);
        }

        if (!modifier.null_or_empty())
        {
            str_builder.append(modifier);
        }

        return str_builder.finalize();
    }

    RefPtr<Element> build() override
    {
        _text_editor->focus();

        return vflow({_titlebar,
                      toolbar({
                          basic_button(Graphic::Icon::get("folder-open"), [&] {
                              on_load();
                          }),
                          basic_button(Graphic::Icon::get("content-save"), [&] {
                              on_save();
                          }),
                          basic_button(Graphic::Icon::get("image-plus")),
                      }),
                      fill(_text_editor)});
    }
};
