// =============================
//  BST VISUALIZER - FULL ANIMATED VERSION
//  (Insert, Search, Delete, Traversals w/ cursor + edge highlight)
//  Clean professional style
// =============================

#include "raylib.h"
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

// ------------------------------
// Node structure
// ------------------------------
struct Node {
    int value;
    Node* left;
    Node* right;

    float x, y;      // current position
    float tx, ty;    // target position
    float alpha = 1;
    Color color = LIGHTGRAY;
};

Node* root = nullptr;

// ------------------------------
// UI Elements
// ------------------------------
struct Button {
    Rectangle bounds;
    const char* text;
};

bool Hover(Button b) {
    return CheckCollisionPointRec(GetMousePosition(), b.bounds);
}

bool Click(Button b) {
    return Hover(b) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

// ------------------------------
// Global animation/state variables
// ------------------------------
int inputValue = 10;
Node* current = nullptr;
bool found = false;

enum Mode {
    IDLE,
    INSERTING,
    SEARCHING,
    DELETING,
    TRAVERSING_IN,
    TRAVERSING_PRE,
    TRAVERSING_POST
};

Mode mode = IDLE;

// ------------------------------
// Algorithm panel state
// ------------------------------
std::vector<std::string> algoSteps;
int algoStepIndex = 0;
float algoSpeed = 0.5f;
float algoTimer = 0;

// ------------------------------
// Layout constants
// ------------------------------
const int PANEL_WIDTH = 260;
const int TREE_START_X = 350;
const int TREE_START_Y = 80;

// ------------------------------
// Utility
// ------------------------------
float lerp(float a, float b, float t) { return a + (b - a) * t; }

// ------------------------------
// Create BST node
// ------------------------------
Node* CreateNode(int v) {
    Node* n = new Node();
    n->value = v;
    n->left = n->right = nullptr;

    n->x = TREE_START_X;
    n->y = -100;
    n->tx = TREE_START_X;
    n->ty = TREE_START_Y;

    return n;
}

// ------------------------------
// BST insert (logical only)
// ------------------------------
Node* Insert(Node* r, int v) {
    if (!r) return CreateNode(v);
    if (v < r->value) r->left = Insert(r->left, v);
    else if (v > r->value) r->right = Insert(r->right, v);
    return r;
}

// ------------------------------
Node* MinNode(Node* r) {
    while (r && r->left) r = r->left;
    return r;
}

// ------------------------------
// Delete logic
// ------------------------------
Node* DeleteNode(Node* r, int v) {
    if (!r) return nullptr;

    if (v < r->value) r->left = DeleteNode(r->left, v);
    else if (v > r->value) r->right = DeleteNode(r->right, v);
    else {
        if (!r->left && !r->right) { delete r; return nullptr; }
        if (!r->left) { Node* t = r->right; delete r; return t; }
        if (!r->right) { Node* t = r->left; delete r; return t; }
        Node* t = MinNode(r->right);
        r->value = t->value;
        r->right = DeleteNode(r->right, t->value);
    }
    return r;
}

// ------------------------------
// Auto layout
// ------------------------------
void SetPositions(Node* r, int x, int y, int offset) {
    if (!r) return;
    r->tx = x;
    r->ty = y;
    SetPositions(r->left, x - offset, y + 80, offset / 2);
    SetPositions(r->right, x + offset, y + 80, offset / 2);
}

// ------------------------------
// Smooth movement
// ------------------------------
void AnimateNodes(float dt) {
    if (!root) return;

    Node* stack[100];
    int top = 0;
    stack[top++] = root;

    while (top > 0) {
        Node* n = stack[--top];

        n->x = lerp(n->x, n->tx, dt * 5);
        n->y = lerp(n->y, n->ty, dt * 5);

        if (n->left)  stack[top++] = n->left;
        if (n->right) stack[top++] = n->right;
    }
}

// ------------------------------
// Draw tree
// ------------------------------
void DrawTree(Node* r, Node* highlight, Node* edgeA, Node* edgeB) {
    if (!r) return;

    // Left edge
    if (r->left) {
        Color c = (edgeA == r && edgeB == r->left) ? RED : YELLOW;
        DrawLine(r->x, r->y, r->left->x, r->left->y, c);
    }

    // Right edge
    if (r->right) {
        Color c = (edgeA == r && edgeB == r->right) ? RED : BLUE;
        DrawLine(r->x, r->y, r->right->x, r->right->y, c);
    }

    DrawTree(r->left, highlight, edgeA, edgeB);
    DrawTree(r->right, highlight, edgeA, edgeB);

    Color c = r->color;
    if (r == highlight) c = ORANGE;
    if (found && r == highlight) c = GREEN;

    DrawCircle(r->x, r->y, 25, c);
    DrawCircleLines(r->x, r->y, 25, BLACK);
    DrawText(std::to_string(r->value).c_str(), r->x - 10, r->y - 10, 20, BLACK);
}

// ------------------------------
// Draw algorithm panel
// ------------------------------
void DrawAlgorithmPanel() {
    int x = 1000 - PANEL_WIDTH;

    DrawRectangle(x, 0, PANEL_WIDTH, 700, Fade(LIGHTGRAY, 0.55f));
    DrawText("Algorithm Steps", x + 20, 20, 22, BLACK);

    float y = 80;
    for (int i = 0; i <= algoStepIndex && i < algoSteps.size(); i++) {
        DrawText(("- " + algoSteps[i]).c_str(), x + 20, y, 19, DARKBLUE);
        y += 30;
    }
}

// =====================================================
// SECTION 2 — Insert / Search / Delete Animations
// =====================================================

// ----------------------------------------------
// INSERT animation start
// ----------------------------------------------
void StartInsertAnimation() {
    mode = INSERTING;

    algoSteps = {
        "Start at root",
        "Compare values",
        "Move left / right",
        "Insert at leaf",
        "Recalculate layout"
    };

    algoStepIndex = 0;
    algoTimer = 0;
    current = root;
}

// ----------------------------------------------
// SEARCH animation start
// ----------------------------------------------
void StartSearchAnimation() {
    mode = SEARCHING;
    found = false;
    current = root;

    algoSteps = {
        "Start at root",
        "Compare target",
        "Move left or right",
        "Repeat until found or NULL"
    };

    algoStepIndex = 0;
    algoTimer = 0;
}

// ----------------------------------------------
// SEARCH animation update
// ----------------------------------------------
void UpdateSearch(float dt) {
    if (!current) {
        mode = IDLE;
        return;
    }

    algoTimer += dt;
    if (algoTimer > algoSpeed && algoStepIndex < algoSteps.size() - 1) {
        algoStepIndex++;
        algoTimer = 0;
    }

    static float moveTimer = 0;
    moveTimer += dt;

    if (moveTimer < 0.6f) return;
    moveTimer = 0;

    if (inputValue == current->value) {
        found = true;
        mode = IDLE;
        return;
    }

    if (inputValue < current->value)
        current = current->left;
    else
        current = current->right;
}

// ----------------------------------------------
// DELETE ANIMATION VARIABLES
// ----------------------------------------------
Node* delNode = nullptr;
float delFlashTimer = 0;
int delFlashCount = 0;

// ----------------------------------------------
// DELETE animation start
// ----------------------------------------------
void StartDeleteAnimation() {
    mode = DELETING;
    delNode = nullptr;
    delFlashTimer = 0;
    delFlashCount = 0;

    algoSteps = {
        "Find node",
        "Flash target node",
        "Drop node",
        "Fade node",
        "Delete & restructure"
    };

    algoStepIndex = 0;
    algoTimer = 0;

    Node* temp = root;
    while (temp) {
        if (temp->value == inputValue) {
            delNode = temp;
            break;
        }
        if (inputValue < temp->value) temp = temp->left;
        else temp = temp->right;
    }

    if (!delNode) {
        mode = IDLE;
    }
}

// ----------------------------------------------
// DELETE animation update
// ----------------------------------------------
void UpdateDelete(float dt) {
    if (!delNode) {
        mode = IDLE;
        return;
    }

    algoTimer += dt;
    if (algoTimer > algoSpeed && algoStepIndex < algoSteps.size() - 1) {
        algoStepIndex++;
        algoTimer = 0;
    }

    // Flash red
    if (algoStepIndex == 1) {
        delFlashTimer += dt;
        if (delFlashTimer > 0.12f) {
            delFlashTimer = 0;
            delFlashCount++;
            delNode->color = (delFlashCount % 2 == 0 ? RED : LIGHTGRAY);
        }
        if (delFlashCount > 6)
            algoStepIndex = 2;
        return;
    }

    // Drop
    if (algoStepIndex == 2) {
        delNode->y += dt * 300;
        if (delNode->y > 900)
            algoStepIndex = 3;
        return;
    }

    // Fade
    if (algoStepIndex == 3) {
        delNode->alpha -= dt * 3;
        if (delNode->alpha <= 0)
            algoStepIndex = 4;
        return;
    }

    // Actual deletion
    if (algoStepIndex == 4) {
        root = DeleteNode(root, inputValue);
        SetPositions(root, TREE_START_X, TREE_START_Y, 200);
        mode = IDLE;
        return;
    }
}


// =====================================================
// SECTION 3 — FULL TRAVERSAL ENGINE (CURSOR + EDGE HIGHLIGHT)
// =====================================================

// Traversal stack frame
struct TravFrame {
    Node* node;
    int state;
    // 0 = go left
    // 1 = visit
    // 2 = go right
    // 3 = return
};

std::vector<TravFrame> travStack;

Node* travHighlight = nullptr;     // active node
Node* edgeA = nullptr;             // edge highlight start
Node* edgeB = nullptr;             // edge highlight end
float travTimer = 0;

// ----------------------------------------------
// Reset highlight
// ----------------------------------------------
void ResetTreeColors() {
    if (!root) return;

    Node* st[100];
    int top = 0;
    st[top++] = root;

    while (top > 0) {
        Node* n = st[--top];
        n->color = LIGHTGRAY;

        if (n->left) st[top++] = n->left;
        if (n->right) st[top++] = n->right;
    }
}

// ----------------------------------------------
// Start In-order Traversal
// ----------------------------------------------
void StartTraverseIn() {
    mode = TRAVERSING_IN;
    travStack.clear();
    travStack.push_back({ root, 0 });
    travHighlight = nullptr;
    edgeA = edgeB = nullptr;

    algoSteps = {
        "In-order traversal:",
        "Go Left",
        "Visit Node",
        "Go Right"
    };

    algoStepIndex = 0;
    algoTimer = 0;
    travTimer = 0;
    ResetTreeColors();
}

// ----------------------------------------------
// Start Pre-order Traversal
// ----------------------------------------------
void StartTraversePre() {
    mode = TRAVERSING_PRE;
    travStack.clear();
    travStack.push_back({ root, 0 });
    travHighlight = nullptr;
    edgeA = edgeB = nullptr;

    algoSteps = {
        "Pre-order traversal:",
        "Visit Node",
        "Go Left",
        "Go Right"
    };

    algoStepIndex = 0;
    algoTimer = 0;
    travTimer = 0;
    ResetTreeColors();
}

// ----------------------------------------------
// Start Post-order Traversal
// ----------------------------------------------
void StartTraversePost() {
    mode = TRAVERSING_POST;
    travStack.clear();
    travStack.push_back({ root, 0 });
    travHighlight = nullptr;
    edgeA = edgeB = nullptr;

    algoSteps = {
        "Post-order traversal:",
        "Go Left",
        "Go Right",
        "Visit Node"
    };

    algoStepIndex = 0;
    algoTimer = 0;
    travTimer = 0;
    ResetTreeColors();
}

// ----------------------------------------------
// TRAVERSAL UPDATE
// ----------------------------------------------
void UpdateTraversal(float dt) {
    if (travStack.empty()) {
        mode = IDLE;
        travHighlight = nullptr;
        edgeA = edgeB = nullptr;
        return;
    }

    algoTimer += dt;
    if (algoTimer > algoSpeed && algoStepIndex < algoSteps.size() - 1) {
        algoStepIndex++;
        algoTimer = 0;
    }

    travTimer += dt;
    if (travTimer < 0.8f) return;
    travTimer = 0;

    TravFrame& frame = travStack.back();
    Node* n = frame.node;

    edgeA = edgeB = nullptr;

    switch (mode) {

        // ==========================================================
        // IN-ORDER TRAVERSAL
        // ==========================================================
    case TRAVERSING_IN:

        if (frame.state == 0) {     // go left
            travHighlight = n;
            if (n->left) {
                edgeA = n;
                edgeB = n->left;
                travStack.push_back({ n->left, 0 });
            }
            frame.state = 1;
            return;
        }

        if (frame.state == 1) {     // visit
            travHighlight = n;
            n->color = ORANGE;
            frame.state = 2;
            return;
        }

        if (frame.state == 2) {     // go right
            if (n->right) {
                edgeA = n;
                edgeB = n->right;
                travStack.push_back({ n->right, 0 });
            }
            frame.state = 3;
            return;
        }

        if (frame.state == 3) {     // return
            travStack.pop_back();
            return;
        }
        break;

        // ==========================================================
        // PRE-ORDER TRAVERSAL
        // ==========================================================
    case TRAVERSING_PRE:

        if (frame.state == 0) {     // visit first
            travHighlight = n;
            n->color = ORANGE;
            frame.state = 1;
            return;
        }

        if (frame.state == 1) {     // go left
            if (n->left) {
                edgeA = n;
                edgeB = n->left;
                travStack.push_back({ n->left, 0 });
            }
            frame.state = 2;
            return;
        }

        if (frame.state == 2) {     // go right
            if (n->right) {
                edgeA = n;
                edgeB = n->right;
                travStack.push_back({ n->right, 0 });
            }
            frame.state = 3;
            return;
        }

        if (frame.state == 3) {     // return
            travStack.pop_back();
            return;
        }
        break;

        // ==========================================================
        // POST-ORDER TRAVERSAL
        // ==========================================================
    case TRAVERSING_POST:

        if (frame.state == 0) {     // go left
            if (n->left) {
                edgeA = n;
                edgeB = n->left;
                travStack.push_back({ n->left, 0 });
            }
            frame.state = 1;
            travHighlight = n;
            return;
        }

        if (frame.state == 1) {     // go right
            if (n->right) {
                edgeA = n;
                edgeB = n->right;
                travStack.push_back({ n->right, 0 });
            }
            frame.state = 2;
            travHighlight = n;
            return;
        }

        if (frame.state == 2) {     // visit
            travHighlight = n;
            n->color = ORANGE;
            frame.state = 3;
            return;
        }

        if (frame.state == 3) {     // return
            travStack.pop_back();
            return;
        }
        break;
    }
}


// =====================================================
// SECTION 4 — MAIN LOOP + UI + RENDER
// =====================================================

int main() {

    InitWindow(1000, 700, "BST Visualizer (Animated Traversal, Insert, Search, Delete)");
    SetTargetFPS(60);

    // Buttons
    Button bInsert = { {40,  620, 130, 45}, "INSERT" };
    Button bSearch = { {190, 620, 130, 45}, "SEARCH" };
    Button bDelete = { {340, 620, 130, 45}, "DELETE" };
    Button bReset = { {490, 620, 130, 45}, "RESET" };

    Button bTravIn = { {650, 620, 100, 45}, "IN-ORDER" };
    Button bTravPre = { {760, 620, 100, 45}, "PRE" };
    Button bTravPost = { {870, 620, 100, 45}, "POST" };

    // Main Loop
    while (!WindowShouldClose()) {

        float dt = GetFrameTime();

        // Value modify with arrow keys
        if (IsKeyPressed(KEY_UP))   inputValue++;
        if (IsKeyPressed(KEY_DOWN)) inputValue--;

        // Button actions
        if (Click(bInsert)) {
            root = Insert(root, inputValue);
            SetPositions(root, TREE_START_X, TREE_START_Y, 200);
            StartInsertAnimation();
        }

        if (Click(bSearch)) {
            StartSearchAnimation();
        }

        if (Click(bDelete)) {
            StartDeleteAnimation();
        }

        if (Click(bReset)) {
            root = nullptr;
            current = nullptr;
            travHighlight = nullptr;
            travStack.clear();
            mode = IDLE;
            algoSteps.clear();
            algoStepIndex = 0;
            ResetTreeColors();
        }

        if (Click(bTravIn))  StartTraverseIn();
        if (Click(bTravPre)) StartTraversePre();
        if (Click(bTravPost))StartTraversePost();

        // ------------------------------
        // UPDATE animations
        // ------------------------------
        AnimateNodes(dt);

        switch (mode) {
        case INSERTING:
            algoTimer += dt;
            if (algoTimer > algoSpeed && algoStepIndex < algoSteps.size() - 1) {
                algoStepIndex++;
                algoTimer = 0;
            }
            break;

        case SEARCHING:
            UpdateSearch(dt);
            break;

        case DELETING:
            UpdateDelete(dt);
            break;

        case TRAVERSING_IN:
        case TRAVERSING_PRE:
        case TRAVERSING_POST:
            UpdateTraversal(dt);
            break;

        default:
            break;
        }

        // ------------------------------
        // DRAW
        // ------------------------------
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText("BST VISUALIZER", 340, 20, 32, DARKBLUE);

        DrawText(("Value: " + std::to_string(inputValue)).c_str(),
            760, 450, 24, BLACK);

        DrawText("UP / DOWN to change value",
            740, 500, 16, DARKGRAY);

        // Draw tree
        DrawTree(root, current ? current : travHighlight, edgeA, edgeB);

        // Draw Buttons
        auto DrawButton = [&](Button b) {
            Color c = Hover(b) ? SKYBLUE : LIGHTGRAY;
            DrawRectangleRec(b.bounds, c);
            DrawRectangleLinesEx(b.bounds, 2, BLACK);
            int tw = MeasureText(b.text, 20);
            DrawText(b.text,
                b.bounds.x + (b.bounds.width - tw) / 2,
                b.bounds.y + 12,
                20, BLACK);
            };

        DrawButton(bInsert);
        DrawButton(bSearch);
        DrawButton(bDelete);
        DrawButton(bReset);

        DrawButton(bTravIn);
        DrawButton(bTravPre);
        DrawButton(bTravPost);

        // Algorithm Panel
        DrawAlgorithmPanel();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
