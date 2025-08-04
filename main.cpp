#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfx.h>
#include <regex>
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <cstring>
#include <fstream>
#include <windows.h>

// SCREEN DIMENSION
const int SCREEN_WIDTH = 1300;
const int SCREEN_HEIGHT = 700;

// STRUCT FOR COLORS OF IDE
struct ThemeColors {
    SDL_Color background;
    SDL_Color menuBarItems;
    SDL_Color menuDropRect;
    SDL_Color treeMap;
    SDL_Color defaultText;
};

const ThemeColors DARK_MODE = {
        /* background   = */  {  0,  0,  0,255},
        /* menuBarItems = */  { 70, 70, 70,255},
        /* menuDropRect = */  { 80, 80, 80,255},
        /* treeMap      = */  { 50, 50, 50,255},
        /* defaultText  = */  {255,255,255,255}
};
const ThemeColors LIGHT_MODE = {
        /* background   = */  {255,255,255,255},
        /* menuBar      = */  {180,180,180,255},
        /* menuDropRect = */  {200,200,200,255},
        /* treeMap      = */  {150,150,150,255},
        /* defaultText  = */  {  0,  0,  0,255}
};

bool haveError = false;


// STRUCT FOR MENU BAR
struct Menu {
    bool fileOpenMenu = false;
    bool editOpenMenu = false;
    bool themeOpenMenu = false;

    // File rectangle in menu
    SDL_Rect fileMenuButton = {246,0,155,35};
    SDL_Rect dropRectFile = {246,35,170,150};
    SDL_Rect itemProjectNew = {246,35,170,50};
    SDL_Rect itemProjectSave = {246,85,170,50};
    SDL_Rect itemExit = {246,135,170,50};

    // Edit rectangle in menu
    SDL_Rect editMenuButton = {401,0,159,35};
    SDL_Rect dropRectEdit = {421,35,120,100};
    SDL_Rect itemUndo = {421,35,120,50};
    SDL_Rect itemRedo = {421,85,120,50};

    // Theme rectangle in menu
    SDL_Rect themeMenuButton = {560,0,175,35};
    SDL_Rect dropRectTheme = {560,35,175,100};
    SDL_Rect itemLight = {560,35,175,50};
    SDL_Rect itemDark = {560,85,175,50};

    // Debug & Compile rectangle in menu
    SDL_Rect dNcMenuButton = {735,0,215,35};

    // Run rectangle in menu
    SDL_Rect runMenuButton = {950,0,175,35};

    // Save rectangle in menu
    SDL_Rect saveMenuButton = {1125,0,175,35};
};

// FUNCTIONS
void ensureLastLineVisible(int, int &, int, int, int);
void librariesError(SDL_Renderer*, const std::vector<std::string>&, TTF_Font*);
void makeTextInRectangle(SDL_Renderer*, TTF_Font*, SDL_Rect&, const std::string&, SDL_Color);
std::string wholeTextFile(const std::vector<std::string>&);
bool isMouseInRect(int, int, const SDL_Rect&);
void renderMenu(SDL_Renderer* , TTF_Font* , Menu&, bool&);
void drawRect(SDL_Renderer*, const SDL_Rect&, SDL_Color);
void saveFile(const std::string&, const std::vector<std::string>&);
void loadFile(const std::string&, std::vector<std::string>&, int&);
void treeMap(SDL_Renderer*, bool&,TTF_Font *, const std::vector<std::string>&);
bool compile(const std::string &);
void runInAnotherWindow(const std::string &);
std::string newProject(SDL_Renderer *renderer,TTF_Font *font);
std::vector<std::string> getFilesInDirectory(const std::string& directoryPath);
void handleTextSelection(SDL_Event&,std::vector<std::string>&, int&, int&, int&, int&, int&, int&, bool&);
void copySelectedText(const std::vector<std::string>&, int&, int&, int&, int&, int&, int&, std::string&);
void pasteText(std::vector<std::string>&, int&, int&, std::string&);
void saveAsOption(SDL_Renderer*,TTF_Font*, const std::vector<std::string>&);
void thereIsError(SDL_Renderer* renderer,TTF_Font* font);
// FUNCTIONS


// MAIN
int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    // Initialize SDL_ttf
    if (TTF_Init() == -1) {
        std::cerr << "TTF could not initialize! TTF_Error: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    // Create window
    SDL_Window* window = SDL_CreateWindow("SDL Text Editor",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          SCREEN_WIDTH,
                                          SCREEN_HEIGHT,
                                          SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    // Create renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    // Load fonts
    TTF_Font* font1 = TTF_OpenFont(R"(C:\Windows\Fonts\consola.ttf)", 19); // Replace with the path to your .ttf font
    if (!font1) {
        std::cerr << "Failed to load font! TTF_Error: " << TTF_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }
    TTF_Font* font2 = TTF_OpenFont(R"(C:\Windows\Fonts\unispace bd.ttf)", 19);
    if (!font2) {
        std::cerr << "Failed to load font! TTF_Error: " << TTF_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }
    TTF_Font* font = font1;
    // Load font for menu bar
    TTF_Font* fontMenuBar = TTF_OpenFont(R"(C:\Windows\Fonts\Arial.ttf)", 23);
    if (!fontMenuBar) {
        std::cerr << "Failed to load font! TTF_Error: " << TTF_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }


    // Read files in a directory
    std::string directoryPath = "C:\\Users\\parsa\\Documents\\university\\C++\\sharifBPBaby403\\ProjectOfBP\\cmake-build-debug";
    std::vector<std::string> filenames = getFilesInDirectory(directoryPath);
    bool isSaved = false;

    std::vector<std::string> lines = {""}; // Holds multiple lines of text
    int currentLine = 0; // Track the current line being edited
    int cursorPos = 0; // Track the cursor position within the current line
    int scrollOffset = 0; // Keeps track of scrolling
    const int LINE_HEIGHT = TTF_FontHeight(font); // Height of each line

    // Timer for cursor blinking
    Uint32 lastCursorToggle = SDL_GetTicks();
    bool cursorVisible = true;
    const Uint32 CURSOR_BLINK_INTERVAL = 500; // 500 ms for blinking

    // Dark or Light
    bool isDark = false;

    // Checking ctrl is down or not
    bool ctrlDown = false;

    // Structure for Menu Bar
    Menu menu;

    // Variables for handling clipboard and ctrl+c and ctrl+x
    int selectionStartX = -1;
    int selectionStartY = -1;
    int selectionEndX = -1;
    int selectionEndY = -1;
    bool shiftHeld = false;
    std::string clipboard;

    // Handling the Events
    SDL_Event e;
    bool quit = false;

    while (!quit) {
        // Handle cursor blinking
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime > lastCursorToggle + CURSOR_BLINK_INTERVAL) {
            cursorVisible = !cursorVisible;
            lastCursorToggle = currentTime;
        }

        // Event loop
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }


            // MENU BAR ---------------------------------------------------------------------------
            else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mx = e.button.x;
                int my = e.button.y;

                // ------------------------File------------------------
                if (!menu.fileOpenMenu && isMouseInRect(mx,my,menu.fileMenuButton)) {
                    menu.fileOpenMenu = true;
                }
                else if (menu.fileOpenMenu) {

                    // Project New
                    if (isMouseInRect(mx,my,menu.itemProjectNew)) {
                        isSaved = false;
                        SDL_SetRenderDrawColor(renderer,255,255,255,255);
                        SDL_RenderClear(renderer);
                        lines.clear();
                        lines = {""};
                        currentLine = 0;
                        cursorPos = 0;
                        menu.fileOpenMenu = false;
                    }

                        // Project Save
                    else if (isMouseInRect(mx,my,menu.itemProjectSave)) {
                        if (isSaved)
                            saveFile(*(filenames.end() - 1),lines);
                        else {
                            filenames.push_back(newProject(renderer,fontMenuBar));
                            saveFile(*(filenames.end() - 1),lines);
                            isSaved = true;
                        }
                        menu.fileOpenMenu = false;
                    }

                        // Exit
                    else if (isMouseInRect(mx,my,menu.itemExit)) {
                        quit = true;
                        menu.fileOpenMenu = false;
                    }
                    else if (!isMouseInRect(mx,my,menu.dropRectFile)) {
                        menu.fileOpenMenu = false;
                    }
                }

                // ------------------------Edit------------------------
                if (!menu.editOpenMenu && isMouseInRect(mx,my,menu.editMenuButton)) {
                    menu.editOpenMenu = true;
                }
                else if (menu.editOpenMenu) {

                    // Redo
                    if (isMouseInRect(mx,my,menu.itemRedo)) {
                        menu.editOpenMenu = false;
                    }

                    // Undo
                    else if (isMouseInRect(mx,my,menu.itemUndo)) {
                        menu.editOpenMenu = false;
                    }

                    else if (!isMouseInRect(mx,my,menu.dropRectEdit)) {
                        menu.editOpenMenu = false;
                    }
                }

                // ------------------------Theme------------------------
                if (!menu.themeOpenMenu && isMouseInRect(mx,my,menu.themeMenuButton)) {
                    menu.themeOpenMenu = true;
                }
                else if (menu.themeOpenMenu) {

                    // Light mode
                    if (isMouseInRect(mx, my, menu.itemLight)) {
                        isDark = false;
                        menu.themeOpenMenu = false;
                    }

                    // Dark mode
                    else if (isMouseInRect(mx, my, menu.itemDark)) {
                        isDark = true;
                        menu.themeOpenMenu = false;
                    }
                    else if (!isMouseInRect(mx,my,menu.dropRectTheme)) {
                        menu.themeOpenMenu = false;
                    }
                }

                // ------------------Debug & Compile-------------------
                if (isMouseInRect(mx,my,menu.dNcMenuButton) && !haveError) {
                    if (haveError) {
                        thereIsError(renderer,fontMenuBar);
                    }
                    else {
                        saveFile(*(filenames.end() - 1), lines);
                        if (compile(*(filenames.end() - 1))) {
                            std::cout << "[INFO] Compilation successful!\n";
                        } else
                            std::cout << "[INFO] Compilation failed!\n";
                    }
                }

                // ------------------------Run-------------------------
                if (isMouseInRect(mx,my,menu.runMenuButton)) {
                    if (haveError) {
                        thereIsError(renderer,fontMenuBar);
                    }
                    else {
                        saveFile(*(filenames.end() - 1), lines);
                        if (compile(*(filenames.end() - 1))) {
                            std::cout << "[INFO] Compilation successful!\n";
                            runInAnotherWindow("output");
                        } else
                            std::cout << "[INFO] Compilation failed!\n";
                    }
                }

                // ------------------------Save------------------------
                if (isMouseInRect(mx,my,menu.saveMenuButton)) {
                    if (isSaved)
                        saveFile(*(filenames.end() - 1),lines);
                    else {
                        filenames.push_back(newProject(renderer,fontMenuBar));
                        saveFile(*(filenames.end() - 1),lines);
                        isSaved = true;
                    }
                }
            }


            else if (e.type == SDL_MOUSEWHEEL) {
                // Handle scroll
                if (e.wheel.y > 0) { // Scroll up
                    scrollOffset = std::max(0, scrollOffset - LINE_HEIGHT);
                }
                else if (e.wheel.y < 0) { // Scroll down
                    scrollOffset += LINE_HEIGHT;
                }
            }

            else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_BACKSPACE) {
                    // Ensure cursorPos is within the valid range
                    if (cursorPos > 0 && cursorPos <= lines[currentLine].size()) {
                        // Remove character before cursor
                        lines[currentLine].erase(cursorPos - 1, 1);
                        cursorPos--;
                    }
                    else if (currentLine > 0) {
                        // Merge with previous line
                        cursorPos = lines[currentLine - 1].size();
                        lines[currentLine - 1] += lines[currentLine].substr(0, lines[currentLine].size());
                        lines.erase(lines.begin() + currentLine);
                        currentLine--;
                    }

                    // Ensure there's always at least one line
                    if (lines.empty()) {
                        lines.push_back("");
                        currentLine = 0;
                        cursorPos = 0;
                    }
                }

                else if (e.key.keysym.sym == SDLK_RETURN) {
                    if (cursorPos <= lines[currentLine].size()) {
                        std::string remainder = lines[currentLine].substr(cursorPos);
                        lines[currentLine] = lines[currentLine].substr(0, cursorPos);
                        lines.insert(lines.begin() + currentLine + 1, remainder);
                        currentLine++;
                        cursorPos = 0;
                        ensureLastLineVisible(currentLine, scrollOffset, SCREEN_HEIGHT, LINE_HEIGHT, lines.size());
                    }
                }

                else if (e.key.keysym.sym == SDLK_TAB) {
                    // Add spaces for tab
                    lines[currentLine].insert(cursorPos, "    ");
                    cursorPos+= 4;
                }

                else if (e.key.keysym.sym == SDLK_LEFT) {
                    // Move cursor left
                    if (cursorPos > 0) {
                        cursorPos--;
                    }
                    else if (currentLine > 0) {
                        currentLine--;
                        cursorPos = lines[currentLine].size();
                    }
                }

                else if (e.key.keysym.sym == SDLK_RIGHT) {
                    // Move cursor right
                    if (cursorPos < lines[currentLine].size()) {
                        cursorPos++;
                    }
                    else if (currentLine < lines.size() - 1) {
                        currentLine++;
                        cursorPos = 0;
                    }
                }

                else if (e.key.keysym.sym == SDLK_UP) {
                    // Move cursor up
                    if (currentLine > 0) {
                        currentLine--;
                        cursorPos = std::min(cursorPos, (int)lines[currentLine].size());
                        ensureLastLineVisible(currentLine, scrollOffset, SCREEN_HEIGHT, LINE_HEIGHT, lines.size());
                    }
                }

                else if (e.key.keysym.sym == SDLK_DOWN) {
                    if (currentLine < lines.size() - 1) {
                        currentLine++;
                        cursorPos = std::min(cursorPos, (int)lines[currentLine].size());
                        ensureLastLineVisible(currentLine, scrollOffset, SCREEN_HEIGHT, LINE_HEIGHT, lines.size());
                    }
                }

                    // Ctrl handling
                else if (e.key.keysym.mod & KMOD_CTRL) {

                        // ctrl+s  Saving
                    if (e.key.keysym.sym == SDLK_s) {
                        saveAsOption(renderer,font,lines);
                    }
                        // Ctrl+c copying
                    else if (e.key.keysym.sym == SDLK_c) {
                        copySelectedText(lines,selectionStartX,selectionEndX,selectionEndY,selectionStartY,cursorPos,currentLine,clipboard);
                    }
                        // Ctrl+v pasting
                    else if (e.key.keysym.sym == SDLK_v) {
                        pasteText(lines,cursorPos,currentLine,clipboard);
                    }

                        // Changing Font
                    else if (e.key.keysym.sym == SDLK_F10) {
                        font = font == font1 ? font2:font1;
                    }
                }

                else if (e.key.keysym.sym == SDLK_LSHIFT || e.key.keysym.sym == SDLK_RSHIFT)
                    handleTextSelection(e,lines,selectionStartX,selectionEndX,selectionEndY,selectionStartY,cursorPos,currentLine,shiftHeld);

            }

            else if (e.type == SDL_TEXTINPUT) {
                if (e.text.text) {
                    lines[currentLine].insert(cursorPos, e.text.text);
                    cursorPos += strlen(e.text.text);
                    ensureLastLineVisible(currentLine, scrollOffset, SCREEN_HEIGHT, LINE_HEIGHT, lines.size());
                }
            }
        }

        // Clear screen
        SDL_Color textColor;
        if (isDark) {
            SDL_SetRenderDrawColor(renderer, DARK_MODE.background.r, DARK_MODE.background.g, DARK_MODE.background.b,
                                   DARK_MODE.background.a);
            textColor = DARK_MODE.defaultText;
        }
        else if (!isDark) {
            SDL_SetRenderDrawColor(renderer, LIGHT_MODE.background.r, LIGHT_MODE.background.g, LIGHT_MODE.background.b,
                                   LIGHT_MODE.background.a);
            textColor = LIGHT_MODE.defaultText;
        }
        SDL_RenderClear(renderer);

        int y = -scrollOffset; // Start rendering based on the scroll offset

        for (size_t j = 0; j < lines.size(); ++j) {
            if (y + LINE_HEIGHT > 0 && y < SCREEN_HEIGHT) { // Render only visible lines
                if (lines[j].empty()) {
                    lines[j] = " "; // Show cursor on the current line
                }
                SDL_Surface* wordSurface = TTF_RenderText_Blended(font, lines[j].c_str(), textColor);
                SDL_Texture* wordTexture = SDL_CreateTextureFromSurface(renderer, wordSurface);

                int wordWidth = wordSurface->w;
                int wordHeight = wordSurface->h;
                SDL_Rect renderQuad = {255, y+44, wordWidth, wordHeight};

                SDL_FreeSurface(wordSurface);

                SDL_RenderCopy(renderer, wordTexture, nullptr, &renderQuad);
                SDL_DestroyTexture(wordTexture);

                // Render cursor if this is the current line
                if (j == currentLine) {
                    int cursorX = 0;
                    if (cursorPos > 0) {
                        TTF_SizeText(font, lines[j].substr(0, cursorPos).c_str(), &cursorX, nullptr);
                    }
                    cursorX += 255; // Add padding for the left margin
                    if (isDark)
                        SDL_SetRenderDrawColor(renderer, DARK_MODE.defaultText.r, DARK_MODE.defaultText.g,
                                               DARK_MODE.defaultText.b, DARK_MODE.defaultText.a);
                    else if (!isDark)
                        SDL_SetRenderDrawColor(renderer, LIGHT_MODE.defaultText.r, LIGHT_MODE.defaultText.g,
                                               LIGHT_MODE.defaultText.b, LIGHT_MODE.defaultText.a);
                    SDL_RenderDrawLine(renderer, cursorX, y+44, cursorX, y + LINE_HEIGHT + 44);
                }
            }
            y += LINE_HEIGHT; // Move to the next line
        }

        // Library Errors
        librariesError(renderer,lines,fontMenuBar);

        // TreeMap
        treeMap(renderer,isDark,fontMenuBar,filenames);

        // Rendering menu
        renderMenu(renderer,fontMenuBar,menu,isDark);

        SDL_RenderPresent(renderer);
    }


    // Clean up
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}

// Ensure the current line is visible when adding new lines or moving the cursor----------------------------------------
void ensureLastLineVisible(int currentLine, int &scrollOffset, int SCREEN_HEIGHT, int LINE_HEIGHT, int totalLines) {
    int cursorY = currentLine * LINE_HEIGHT - scrollOffset;
    if (cursorY < 0) {
        // Scroll up
        scrollOffset = currentLine * LINE_HEIGHT;
    } else if (cursorY + LINE_HEIGHT > SCREEN_HEIGHT) {
        // Scroll down
        scrollOffset = (currentLine + 1) * LINE_HEIGHT - SCREEN_HEIGHT;
    }

    // Ensure last line is always visible
    int contentHeight = totalLines * LINE_HEIGHT;
    if (contentHeight > SCREEN_HEIGHT) {
        scrollOffset = std::min(scrollOffset, contentHeight - SCREEN_HEIGHT);
    } else {
        scrollOffset = 0; // No scrolling needed if content fits
    }
}

// CHECKING LIBRARIES ERROR---------------------------------------------------------------------------------------------
void librariesError(SDL_Renderer* renderer,const std::vector<std::string>& lines,TTF_Font* font) {
    // gathering whole vector string to a single string
    std::string wholeText = wholeTextFile(lines);

    // Regex directions
    std::regex bits("#include <bits/stdc++.h>");
    std::regex iostream("#include <iostream>");
    std::regex notIostream("std::(sqrt|pow|sin|cos|tan|abs|exp|log|log10|floor|ceil)");
    std::regex cmath("#include <cmath>");
    std::regex notCmath("std::(cout|cin|endl|getline)");

    // Rectangle of Error
    SDL_Rect error = {255,660,850,40};

    // CKECK IOSTREAM
    if (!regex_search(wholeText,iostream) && !regex_search(wholeText,bits) && regex_search(wholeText,notCmath)){
        makeTextInRectangle(renderer,font,error,"Library Error ... , Include iostream or bits",{255,0,0,255});
        haveError = true;
    }
    else
        haveError = false;

    // CHECK CMATH
    if (!regex_search(wholeText,cmath) && !regex_search(wholeText,bits) && regex_search(wholeText,notIostream)) {
        makeTextInRectangle(renderer,font,error,"Library Error ... , Include cmath or bits",{255,0,0,255});
        haveError = true;
    }
    else
        haveError = false;
}

// MAKING A RECTANGLE WITH TEXT IN A BOX--------------------------------------------------------------------------------
void makeTextInRectangle(SDL_Renderer* renderer,TTF_Font* font,SDL_Rect& renderQuad,const std::string& text,SDL_Color textColor) {
    SDL_Surface* textSurface = TTF_RenderText_Blended(font, text.c_str(), textColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

    int textWidth = textSurface->w;
    int textHeight = textSurface->h;

    SDL_Rect dest = {renderQuad.x + (renderQuad.w - textWidth)/2,
                     renderQuad.y + (renderQuad.h - textHeight)/2,
                     textWidth,textHeight};

    SDL_FreeSurface(textSurface);

    SDL_RenderCopy(renderer, textTexture, nullptr, &dest);
    SDL_DestroyTexture(textTexture);
}

// CONVERTING WHOLE TEXT TO A STRING------------------------------------------------------------------------------------
std::string wholeTextFile(const std::vector<std::string>& lines) {
    std::string result = "";
    for (auto line:lines) {
        result += line + '\n';
    }
    return result;
}

// CHECKING IF MOUSE IS IN THE BOX--------------------------------------------------------------------------------------
bool isMouseInRect(int mouseX, int mouseY, const SDL_Rect& rect) {
    return (mouseX >= rect.x && mouseX < rect.x + rect.w && mouseY >= rect.y && mouseY < rect.y + rect.h);
}

// RENDER MENU BAR------------------------------------------------------------------------------------------------------
void renderMenu(SDL_Renderer* renderer, TTF_Font* font, Menu& menu, bool& isDark) {
    if (isDark) {
        // File button
        drawRect(renderer, menu.fileMenuButton,DARK_MODE.menuBarItems);
        makeTextInRectangle(renderer, font, menu.fileMenuButton, "File", DARK_MODE.defaultText);

        // Edit button
        drawRect(renderer, menu.editMenuButton,DARK_MODE.menuBarItems);
        makeTextInRectangle(renderer, font, menu.editMenuButton, "Edit", DARK_MODE.defaultText);

        // Theme button
        drawRect(renderer, menu.themeMenuButton, DARK_MODE.menuBarItems);
        makeTextInRectangle(renderer, font, menu.themeMenuButton, "Theme", DARK_MODE.defaultText);

        // Debug & Compile button
        drawRect(renderer, menu.dNcMenuButton, DARK_MODE.menuBarItems);
        makeTextInRectangle(renderer, font, menu.dNcMenuButton, "Deb & Com", DARK_MODE.defaultText);

        // Run button
        drawRect(renderer, menu.runMenuButton, DARK_MODE.menuBarItems);
        makeTextInRectangle(renderer, font, menu.runMenuButton, "Run", DARK_MODE.defaultText);

        // File button
        drawRect(renderer, menu.saveMenuButton, DARK_MODE.menuBarItems);
        makeTextInRectangle(renderer, font, menu.saveMenuButton, "Save", DARK_MODE.defaultText);

        if (menu.fileOpenMenu) { // File
            drawRect(renderer,menu.dropRectFile,DARK_MODE.menuDropRect);
            makeTextInRectangle(renderer,font,menu.itemProjectNew,"Project New",DARK_MODE.defaultText);
            makeTextInRectangle(renderer,font,menu.itemProjectSave,"Project Save",DARK_MODE.defaultText);
            makeTextInRectangle(renderer,font,menu.itemExit,"EXIT",DARK_MODE.defaultText);
        }
        else if (menu.editOpenMenu) { // Edit
            drawRect(renderer,menu.dropRectEdit,DARK_MODE.menuDropRect);
            makeTextInRectangle(renderer,font,menu.itemUndo,"Undo",DARK_MODE.defaultText);
            makeTextInRectangle(renderer,font,menu.itemRedo,"Redo",DARK_MODE.defaultText);
        }
        else if (menu.themeOpenMenu) { // Theme
            drawRect(renderer,menu.dropRectTheme,DARK_MODE.menuDropRect);
            makeTextInRectangle(renderer,font,menu.itemDark,"Dark Mode", DARK_MODE.defaultText);
            makeTextInRectangle(renderer,font,menu.itemLight,"Light Mode", DARK_MODE.defaultText);
        }
    }
    else if (!isDark) {
        // File button
        drawRect(renderer, menu.fileMenuButton, LIGHT_MODE.menuBarItems);
        makeTextInRectangle(renderer, font, menu.fileMenuButton, "File", LIGHT_MODE.defaultText);

        // Edit button
        drawRect(renderer, menu.editMenuButton, LIGHT_MODE.menuBarItems);
        makeTextInRectangle(renderer, font, menu.editMenuButton, "Edit", LIGHT_MODE.defaultText);

        // Theme button
        drawRect(renderer, menu.themeMenuButton, LIGHT_MODE.menuBarItems);
        makeTextInRectangle(renderer, font, menu.themeMenuButton, "Theme", LIGHT_MODE.defaultText);

        // Debug & Compile button
        drawRect(renderer, menu.dNcMenuButton, LIGHT_MODE.menuBarItems);
        makeTextInRectangle(renderer, font, menu.dNcMenuButton, "Deb & Com", LIGHT_MODE.defaultText);

        // Run button
        drawRect(renderer, menu.runMenuButton, LIGHT_MODE.menuBarItems);
        makeTextInRectangle(renderer, font, menu.runMenuButton, "Run", LIGHT_MODE.defaultText);

        // File button
        drawRect(renderer, menu.saveMenuButton, LIGHT_MODE.menuBarItems);
        makeTextInRectangle(renderer, font, menu.saveMenuButton, "Save", LIGHT_MODE.defaultText);

        if (menu.fileOpenMenu) { // File
            drawRect(renderer,menu.dropRectFile,LIGHT_MODE.menuDropRect);
            makeTextInRectangle(renderer,font,menu.itemProjectNew,"Project New",LIGHT_MODE.defaultText);
            makeTextInRectangle(renderer,font,menu.itemProjectSave,"Project Save",LIGHT_MODE.defaultText);
            makeTextInRectangle(renderer,font,menu.itemExit,"EXIT",LIGHT_MODE.defaultText);
        }
        else if (menu.editOpenMenu) { // Edit
            drawRect(renderer,menu.dropRectEdit,LIGHT_MODE.menuDropRect);
            makeTextInRectangle(renderer,font,menu.itemUndo,"Undo",LIGHT_MODE.defaultText);
            makeTextInRectangle(renderer,font,menu.itemRedo,"Redo",LIGHT_MODE.defaultText);
        }
        else if (menu.themeOpenMenu) { // Theme
            drawRect(renderer,menu.dropRectTheme,LIGHT_MODE.menuDropRect);
            makeTextInRectangle(renderer,font,menu.itemDark,"Dark Mode", LIGHT_MODE.defaultText);
            makeTextInRectangle(renderer,font,menu.itemLight,"Light Mode", LIGHT_MODE.defaultText);
        }
    }
}

// FUNCTION FOR DRAWING RECTANGLE --------------------------------------------------------------------------------------
void drawRect(SDL_Renderer* renderer, const SDL_Rect& rect, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
}

// Save file -----------------------------------------------------------------------------------------------------------
void saveFile(const std::string& fileName, const std::vector<std::string>& lines) {
    std::ofstream file(fileName);
    if (file.good()) {
        for (auto line : lines) {
            file << line << '\n';
        }
        std::cout << "[INFO] File saved.\n";
    }
    else {
        std::cout << "[ERROR] Could not open file for saving: " << fileName << "\n";
        return;
    }
    file.close();
}

// Load file -----------------------------------------------------------------------------------------------------------
void loadFile(const std::string& fileName, std::vector<std::string>& lines,int& i) {
    std::ifstream file(fileName);
    std::string line;
    if (file.good()) {
        while (!file.eof()) {
            getline(file,line);
            lines.push_back(line);
            i++;
        }
    }
    else {
        std::cout << "[INFO] Could not open file: " << fileName << ". Starting with an empty buffer.\n";
        return;
    }
    file.close();
}

// TREE MAP ------------------------------------------------------------------------------------------------------------
void treeMap(SDL_Renderer* renderer,bool& isDark,TTF_Font *font, const std::vector<std::string>& filenames) {
    SDL_Rect projectText = {0,0,246,60};
        if (isDark) {
            drawRect(renderer, {0, 0, 246, 700}, DARK_MODE.treeMap);
            makeTextInRectangle(renderer, font, projectText, "Projects", DARK_MODE.defaultText);
            if (filenames.empty()) {
                SDL_Rect error = {10, 80, 249, 40};
                makeTextInRectangle(renderer, font, error, "No projects here", DARK_MODE.defaultText);
                return;
            }

            int i = 0;
            for (auto name: filenames) {
                SDL_Rect fileRect = {10, 80 + 40 * i, 220, 20};
                i++;
                makeTextInRectangle(renderer, font, fileRect, name, DARK_MODE.defaultText);
            }
        }
        else if (!isDark) {
            drawRect(renderer, {0, 0, 246, 700}, LIGHT_MODE.treeMap);
            makeTextInRectangle(renderer, font, projectText, "Projects", LIGHT_MODE.defaultText);
            if (filenames.empty()) {
                SDL_Rect error = {10, 80, 249, 40};
                makeTextInRectangle(renderer, font, error, "No projects here", LIGHT_MODE.defaultText);
                return;
            }

            int i = 0;
            for (auto name: filenames) {
                SDL_Rect fileRect = {10, 80 + 40 * i, 220, 20};
                i++;
                makeTextInRectangle(renderer, font, fileRect, name, LIGHT_MODE.defaultText);
            }
        }

}

// Function to compile a C++ source file -------------------------------------------------------------------------------
bool compile(const std::string &filename) {
    std::string compileCommand = "g++ -x c++ " + filename + " -o output"; // Adjust as needed
    int result = std::system(compileCommand.c_str());
    return (result == 0);
}

// Run in another window -----------------------------------------------------------------------------------------------
void runInAnotherWindow(const std::string &filename) {
    std::string runCommand = "start cmd /c \"" + filename + " & pause\"";
    std::system(runCommand.c_str());
}

// Function to "debug" (simulated basic debugging) ---------------------------------------------------------------------
void debug(const std::string &filename) {
    std::cout << "Debugging " << filename << " (simulated):\n";
    std::ifstream file(filename);
    if (file.is_open()) {
        std::string line;
        int lineNumber = 1;
        while (std::getline(file, line)) {
            std::cout << lineNumber << ": " << line << std::endl;
            lineNumber++;
        }
        file.close();
    }
    else {
        std::cerr << "Unable to open file for debugging." << std::endl;
    }
}

// Function to separating the words ------------------------------------------------------------------------------------
std::vector<std::string> splitLineInWords(std::string line) {
    std::vector<std::string> words;
    size_t start = 0;
    std::string ref = " \t";
    size_t end = line.find_first_of(ref,start);
    size_t endo = line.find_first_not_of(ref,end);

    while ((end != std::string::npos) && (endo != std::string::npos)) {
        words.push_back(line.substr(start,endo - start));

        start = endo;
        end = line.find_first_of(ref,start);
        endo = line.find_first_not_of(ref,end);
    }

    if (start < line.length())
        words.push_back(line.substr(start));

    return words;
}

// Function for making a new file
std::string newProject(SDL_Renderer *renderer,TTF_Font *font) {
    SDL_Rect done = {575,340,150,40};
    SDL_Color textColor = {0,0,0,255};

    SDL_Event e;
    std::string filename = " ";
    bool isDone = false;
    while (!isDone) {
        while(SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mx = e.button.x;
                int my = e.button.y;
                if (isMouseInRect(mx, my, done))
                    isDone = true;
            }
            else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_BACKSPACE) {
                    filename.pop_back();
                }
            }
            else if (e.type == SDL_TEXTINPUT) {
                filename += e.text.text;
            }
        }

        drawRect(renderer, {500, 200, 300, 200}, {150, 150, 150, 255});
        SDL_Surface *textSurface = TTF_RenderText_Blended(font, filename.c_str(), textColor);
        SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

        int textWidth = textSurface->w;
        int textHeight = textSurface->h;
        SDL_Rect renderQuad = {520, 220, textWidth, textHeight};

        SDL_FreeSurface(textSurface);

        SDL_RenderCopy(renderer, textTexture, nullptr, &renderQuad);
        SDL_DestroyTexture(textTexture);


        drawRect(renderer, done, {255, 255, 255, 255});
        makeTextInRectangle(renderer, font, done, "Done", {0, 0, 0, 255});
        SDL_RenderPresent(renderer);
    }

    auto iter = filename.begin();
    filename.erase(iter);


    return (filename + ".txt");
}

// Function to get files in a directory (platform-specific)
std::vector<std::string> getFilesInDirectory(const std::string& directoryPath) {
    std::vector<std::string> files;

    // Windows implementation
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile((directoryPath + "\\*").c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to open directory!" << std::endl;
        return files;
    }

    do {
        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            files.push_back(findFileData.cFileName);
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);

    return files;
}

// Handling selected text
void handleTextSelection(SDL_Event& event, std::vector<std::string>& lines,int& selectionStartX,int& selectionEndX,int& selectionEndY,int& selectionStartY,int& cursorX,int& cursorY,bool& shiftHeld) {
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_LEFT) {
            if (cursorX > 0) {
                cursorX--;
            } else if (cursorY > 0) {
                cursorY--;
                cursorX = lines[cursorY].length();
            }
        } else if (event.key.keysym.sym == SDLK_RIGHT) {
            if (cursorX < lines[cursorY].length()) {
                cursorX++;
            } else if (cursorY < lines.size() - 1) {
                cursorY++;
                cursorX = 0;
            }
        }

        if (event.key.keysym.mod & KMOD_SHIFT) {
            if (selectionStartX == -1) {
                selectionStartX = cursorX;
                selectionStartY = cursorY;
            }
            selectionEndX = cursorX;
            selectionEndY = cursorY;
        } else {
            selectionStartX = -1;
            selectionStartY = -1;
            selectionEndX = -1;
            selectionEndY = -1;
        }
    }
}

// copying in clipboard
void copySelectedText(const std::vector<std::string>& lines,int& selectionStartX,int& selectionEndX,int& selectionEndY,int& selectionStartY,int& cursorX,int& cursorY,std::string& clipboard) {
    if (selectionStartX == -1 || selectionEndX == -1)
        return;

    int startY = std::min(selectionStartY, selectionEndY);
    int endY = std::max(selectionStartY, selectionEndY);
    int startX = (startY == selectionStartY) ? selectionStartX : selectionEndX;
    int endX = (endY == selectionEndY) ? selectionEndX : selectionStartX;

    clipboard.clear();

    for (int y = startY; y <= endY; y++) {
        int lineStart = (y == startY) ? startX : 0;
        int lineEnd = (y == endY) ? endX : lines[y].length();

        if (lineStart < lines[y].length() && lineEnd <= lines[y].length()) {
            clipboard += lines[y].substr(lineStart, lineEnd - lineStart);
            if (y != endY)
                clipboard += "\n";
        }
    }

    std::cout << "Copied to clipboard: " << clipboard << std::endl;
}


// pasting text
void pasteText(std::vector<std::string>& lines,int& cursorX,int& cursorY,std::string& clipboard) {
    if (clipboard.empty()) return;

    size_t newlinePos = clipboard.find('\n');
    if (newlinePos == std::string::npos) {
        lines[cursorY].insert(cursorX, clipboard);
        cursorX += clipboard.length();
    } else {
        std::string firstLine = clipboard.substr(0, newlinePos);
        lines[cursorY].insert(cursorX, firstLine);
        cursorX = firstLine.length();

        std::string remainingText = clipboard.substr(newlinePos + 1);
        size_t nextNewlinePos;
        while ((nextNewlinePos = remainingText.find('\n')) != std::string::npos) {
            lines.insert(lines.begin() + cursorY + 1, remainingText.substr(0, nextNewlinePos));
            cursorY++;
            remainingText = remainingText.substr(nextNewlinePos + 1);
        }
        if (!remainingText.empty()) {
            lines.insert(lines.begin() + cursorY + 1, remainingText);
            cursorY++;
            cursorX = remainingText.length();
        }
    }

    std::cout << "Pasted from clipboard: " << clipboard << std::endl;
}

// Ctrl+s Option
void saveAsOption(SDL_Renderer* renderer,TTF_Font* font, const std::vector<std::string>& lines) {
    SDL_Rect done = {575,340,150,40};
    SDL_Color textColor = {0,0,0,255};

    SDL_Event e;
    std::string filename = " ";
    bool isDone = false;
    while (!isDone) {
        while(SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mx = e.button.x;
                int my = e.button.y;
                if (isMouseInRect(mx, my, done))
                    isDone = true;
            }
            else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_BACKSPACE) {
                    filename.pop_back();
                }
            }
            else if (e.type == SDL_TEXTINPUT) {
                filename += e.text.text;
            }
        }

        drawRect(renderer, {500, 200, 300, 200}, {150, 150, 150, 255});
        SDL_Surface *textSurface = TTF_RenderText_Blended(font, filename.c_str(), textColor);
        SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

        int textWidth = textSurface->w;
        int textHeight = textSurface->h;
        SDL_Rect renderQuad = {520, 220, textWidth, textHeight};

        SDL_FreeSurface(textSurface);

        SDL_RenderCopy(renderer, textTexture, nullptr, &renderQuad);
        SDL_DestroyTexture(textTexture);


        drawRect(renderer, done, {255, 255, 255, 255});
        makeTextInRectangle(renderer, font, done, "Done", {0, 0, 0, 255});
        SDL_RenderPresent(renderer);
    }

    auto iter = filename.begin();
    filename.erase(iter);
    filename += ".cpp";

    saveFile(filename,lines);
}

// Errors before compilation
void thereIsError(SDL_Renderer* renderer,TTF_Font* font) {
    SDL_Rect text = {550, 330, 200, 40};
    drawRect(renderer, {500, 300, 300, 100}, {200, 200, 200, 255});
    makeTextInRectangle(renderer, font, text, "There is an Error", {255, 0, 0, 255});
}