//
/// Created by Yohwllo on 2022/07/13
//

#ifndef CREBON_INPUT_HANDLE_HPP
#define CREBON_INPUT_HANDLE_HPP

namespace cr::input_handling
{
    class base
    {
    public:
        cr::component::settings *settings_pointer;

    private:
        void set_settings(cr::component::settings settings_pointer_in)
        {
            settings_pointer = &settings_pointer_in;
        }

        virtual void edit_data()
        {
            
        }

    protected:
    };
}

#endif // CREBON_INPUT_HANDLE_HPP