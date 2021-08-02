#include <gui/main_screen/MainView.hpp>
#include <BitmapDatabase.hpp>
#include <touchgfx/widgets/Image.hpp>
#include <touchgfx/widgets/TextureMapper.hpp>
#include <touchgfx/widgets/AnimationTextureMapper.hpp>
#include <touchgfx/EasingEquations.hpp>
#include <touchgfx/hal/HAL.hpp>

MainView::MainView() :
    tickCounter(0),
    hours(1),
    minutes(10),
    seconds(0),
    humidityEntropi(0.0f),
    pressureEntropi(0.0f)
{
}


void MainView::setupScreen()
{
    HAL::getInstance()->setFrameRateCompensation(true);

    // Set hands to initial positions
    analogClock.setTime24Hour(hours, minutes, seconds);
}

void MainView::tearDownScreen()
{
    HAL::getInstance()->setFrameRateCompensation(false);
}

void MainView::handleTickEvent()
{
    ++tickCounter;

    if (tickCounter % 108 == 0)
    {
        if (++seconds >= 60)
        {
            seconds = 0;
            if (++minutes >= 60)
            {
                minutes = 0;
                if (++hours >= 24)
                {
                    hours = 0;
                }
            }
        }

        // Update humidity
        humidityEntropi = ((seconds * seconds + seconds * 5 + hours) % 180) / 360.0f * 2 * PI - PI / 2;
        humidity.setupAnimation(AnimationTextureMapper::Z_ROTATION, humidityEntropi, 250, 0, EasingEquations::cubicEaseInOut);
        humidity.startAnimation();

        // Update pressure
        pressureEntropi = (((seconds + minutes + hours) * (seconds + minutes + hours)) % 180) / 360.0f * 2 * PI - PI / 2;
        pressure.setupAnimation(AnimationTextureMapper::Z_ROTATION, pressureEntropi, 250, 0, EasingEquations::cubicEaseInOut);
        pressure.startAnimation();

        // Update the clock
        analogClock.setTime24Hour(hours, minutes, seconds);
    }
}
