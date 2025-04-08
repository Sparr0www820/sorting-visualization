#include <SFML/Graphics.hpp>
#include <algorithm>
#include <random>

constexpr unsigned width    = 1280u; // Original window width
constexpr unsigned height   = 960u;  // Original window height
constexpr unsigned num_bars = 10;    // Number of bars (array length)
// TODO: num_bars should be a mutable config option.

constexpr sf::Time delay_time = sf::milliseconds(250); // Pause time on every iteration (of sorting)
constexpr sf::Time swap_time  = sf::milliseconds(500); // Swap animation duration
constexpr sf::Time swap_delay = sf::milliseconds(100);

/**
 * Drawing bars.
 * @param window The sfml render window.
 * @param bars Integer vector to draw.
 * @param sorted Bool vector deciding every element is sorted or not. A bar will be rendered green if sorted.
 * @param highlight1 Highlight index will be red. (Optional)
 * @param highlight2 Highlight index will be red. (Optional)
 * @param special1 Modify highlight1 bar x-position. Needed when doing swap animations. (Optional)
 * @param special2 Modify highlight2 bar x-position. Needed when doing swap animations. (Optional)
 */
void drawBars(
    sf::RenderWindow&        window,
    const std::vector<int>&  bars,
    const std::vector<bool>& sorted,
    const int                highlight1 = -1,
    const int                highlight2 = -1,
    const float              special1   = -1.f,
    const float              special2   = -1.f
) {
    const float width_f   = static_cast<float>(window.getSize().x);
    const float height_f  = static_cast<float>(window.getSize().y);
    const float bar_width = width_f / num_bars;

    const int max_bar = *std::ranges::max_element(bars);

    sf::RectangleShape bar;
    for (int i = 0; i < bars.size(); ++i) {
        const float margin        = width_f * .1f / num_bars;
        const float left_margin   = margin;
        const float right_margin  = margin;
        const float top_margin    = margin;
        const float bottom_margin = margin;
        // bottom_margin currently doesn't do anything, it seems better not having a bottom margin, but I'll leave it here for now

        bar.setSize({
            bar_width - left_margin - right_margin,
            static_cast<float>(bars[i]) / static_cast<float>(max_bar) * height_f - top_margin - bottom_margin
        });
        bar.setOrigin({ bar.getSize().x / 2, bar.getSize().y / 2 });
        bar.setPosition({ static_cast<float>(i) * bar_width + bar_width / 2, height_f - bar.getSize().y / 2 });

        // Adjust position for special1 and special2
        if (i == highlight1 && special1 >= 0.f) {
            bar.setPosition({ special1, height_f - bar.getSize().y / 2 });
        } else if (i == highlight2 && special2 >= 0.f) {
            bar.setPosition({ special2, height_f - bar.getSize().y / 2 });
        }

        // Set outline
        bar.setOutlineThickness(2.f);
        bar.setOutlineColor(sf::Color::Black);

        // Set color
        if (i == highlight1 || i == highlight2) {
            if (special1 >= 0.f || special2 >= 0.f) {
                bar.setFillColor(sf::Color::Red);
            } else {
                bar.setFillColor(sf::Color::Yellow);
            }
        } else {
            bar.setFillColor(sf::Color::White);
        }
        if (sorted[i]) {
            bar.setFillColor(sf::Color::Green);
        }

        window.draw(bar);
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode({ width, height }), "Bubble Sort Visualization");
    window.setFramerateLimit(144);

    /// Generate random array
    std::vector<int>                bars(num_bars);
    std::random_device              rd;
    std::mt19937                    gen(rd());
    std::uniform_int_distribution<> dis(1, 100);
    for (int i = 0; i < num_bars; ++i) {
        bars[i] = dis(gen);
    }
    // TODO: Bars should be editable by user.

    std::vector<bool> sorted(num_bars, false);

    int si = 0, sj = 0; // indexes used in bubble sorting
    // TODO: Bubble sort is currently 'hard-coded'. This should be more extendable.

    bool sorting = true;

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            } else if (event->is<sf::Event::Resized>()) {
                sf::View view(sf::FloatRect(
                    { 0.f, 0.f },
                    sf::Vector2f(window.getSize())
                ));
                window.setView(view);
            }
        }

        const float width_f = static_cast<float>(window.getSize().x);
        // const float height_f  = static_cast<float>(window.getSize().y); // Not needed for now
        const float bar_width = width_f / num_bars;

        window.clear(sf::Color(127, 127, 127));

        // Draw bars
        if (sorting && sj < num_bars - si - 1) {
            drawBars(window, bars, sorted, sj, sj + 1);
        } else {
            drawBars(window, bars, sorted);
        }

        window.display();

        // Handle sorting & swap animation
        if (sorting) {
            if (sj < num_bars - si - 1) {
                sf::sleep(delay_time);
                if (bars[sj] > bars[sj + 1]) {
                    const float     x1       = static_cast<float>(sj) * bar_width + bar_width / 2;
                    const float     x2       = static_cast<float>(sj) * bar_width + bar_width + bar_width / 2;
                    constexpr float duration = swap_time.asMilliseconds();
                    sf::Clock       clock;

                    while (static_cast<float>(clock.getElapsedTime().asMilliseconds()) < duration) {
                        const float t         = static_cast<float>(clock.getElapsedTime().asMilliseconds()) / duration;
                        const float currentX1 = x1 + (x2 - x1) * t;
                        const float currentX2 = x2 - (x2 - x1) * t;

                        // Window should be able to close while playing the animation
                        while (const std::optional event = window.pollEvent()) {
                            if (event->is<sf::Event::Closed>()) {
                                window.close();
                            }
                        }

                        window.clear(sf::Color(127, 127, 127));
                        drawBars(window, bars, sorted, sj, sj + 1, currentX1, currentX2);
                        window.display();
                    }
                    std::swap(bars[sj], bars[sj + 1]);
                }
                sj++;
            } else {
                sorted[sj] = true;

                sj = 0;
                si++;
                sf::sleep(delay_time);
                if (si >= num_bars - 1) {
                    sorted[0] = true;
                    sorting   = false;
                }
            }
        }
    }

    return 0;
}
