#ifndef IM_NODE_FLOW
#define IM_NODE_FLOW
#pragma once

#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <cmath>
#include <memory>
#include <algorithm>
#include <functional>
#include <unordered_map>
#include <imgui.h>
#include "imgui_bezier_math.h"
#include "context_wrapper.h"

//#define ConnectionFilter_None       [](ImFlow::Pin* out, ImFlow::Pin* in){ return true; }
//#define ConnectionFilter_SameType   [](ImFlow::Pin* out, ImFlow::Pin* in){ return out->getDataType() == in->getDataType(); }
//#define ConnectionFilter_Numbers    [](ImFlow::Pin* out, ImFlow::Pin* in){ return out->getDataType() == typeid(double) || out->getDataType() == typeid(float) || out->getDataType() == typeid(int); }

namespace ImFlow
{
    // -----------------------------------------------------------------------------------------------------------------
    // HELPERS

    /**
     * @brief <BR>Draw a sensible bezier between two points
     * @param p1 Starting point
     * @param p2 Ending point
     * @param color Color of the curve
     * @param thickness Thickness of the curve
     */
    inline static void smart_bezier(const ImVec2& p1, const ImVec2& p2, ImU32 color, float thickness);

    /**
     * @brief <BR>Collider checker for smart_bezier
     * @details Projects the point "p" orthogonally onto the bezier curve and
     *          checks if the distance is less than the given radius.
     * @param p Point to be tested
     * @param p1 Starting point of smart_bezier
     * @param p2 Ending point of smart_bezier
     * @param radius Lateral width of the hit box
     * @return [TRUE] if "p" is inside the collider
     *
     * Intended to be used in union with smart_bezier();
     */
    inline static bool smart_bezier_collider(const ImVec2& p, const ImVec2& p1, const ImVec2& p2, float radius);

    // -----------------------------------------------------------------------------------------------------------------
    // CLASSES PRE-DEFINITIONS

    template<typename T> class InPin;
    template<typename T> class OutPin;
    class Pin; class BaseNode;
    class ImNodeFlow; class ConnectionFilter;

    // -----------------------------------------------------------------------------------------------------------------
    // PIN'S PROPERTIES

    typedef unsigned long long int PinUID;

    /**
     * @brief Extra pin's style setting
     */
    struct PinStyleExtras
    {
        /// @brief Top and bottom spacing
        ImVec2 padding = ImVec2(3.f, 1.f);
        /// @brief Border and background corner rounding
        float bg_radius = 8.f;
        /// @brief Border thickness
        float border_thickness = 1.f;
        /// @brief Background color
        ImU32 bg_color = IM_COL32(23, 16, 16, 0);
        /// @brief Background color when hovered
        ImU32 bg_hover_color = IM_COL32(100, 100, 255, 70);
        /// @brief Border color
        ImU32 border_color = IM_COL32(255, 255, 255, 0);

        /// @brief Link thickness
        float link_thickness = 2.6f;
        /// @brief Link thickness when dragged
        float link_dragged_thickness = 2.2f;
        /// @brief Link thickness when hovered
        float link_hovered_thickness = 3.5f;
        /// @brief Thickness of the outline of a selected link
        float link_selected_outline_thickness = 0.5f;
        /// @brief Color of the outline of a selected link
        ImU32 outline_color = IM_COL32(80, 20, 255, 200);

        /// @brief Spacing between pin content and socket
        float socket_padding = 6.6f;

    };

    /**
     * @brief Defines the visual appearance of a pin
     */
    class PinStyle
    {
    public:
        PinStyle() = delete;
        
        constexpr PinStyle(ImU32 color, int socket_shape, float socket_radius, float socket_hovered_radius, float socket_connected_radius, float socket_thickness) noexcept(true)
                :color(color), socket_shape(socket_shape), socket_radius(socket_radius), socket_hovered_radius(socket_hovered_radius), socket_connected_radius(socket_connected_radius),  socket_thickness(socket_thickness) 
        {}

        /// @brief Socket and link color
        ImU32 color;
        /// @brief Socket shape ID
        int socket_shape;
        /// @brief Socket radius
        float socket_radius;
        /// @brief Socket radius when hovered
        float socket_hovered_radius;
        /// @brief Socket radius when connected
        float socket_connected_radius;
        /// @brief Socket outline thickness when empty
        float socket_thickness;
        /// @brief List of less common properties
        PinStyleExtras extra;
    public:
        /// @brief <BR>Default cyan style
        static std::shared_ptr<PinStyle> cyan()  noexcept(true) { return std::make_shared<PinStyle>(PinStyle(IM_COL32(87,155,185,255), 0, 4.f, 4.67f, 3.7f, 1.f)); }
        /// @brief <BR>Default green style
        static std::shared_ptr<PinStyle> green() noexcept(true) { return std::make_shared<PinStyle>(PinStyle(IM_COL32(90,191,93,255), 4, 4.f, 4.67f, 4.2f, 1.3f)); }
        /// @brief <BR>Default blue style
        static std::shared_ptr<PinStyle> blue()  noexcept(true) { return std::make_shared<PinStyle>(PinStyle(IM_COL32(90,117,191,255), 0, 4.f, 4.67f, 3.7f, 1.f)); }
        /// @brief <BR>Default brown style
        static std::shared_ptr<PinStyle> brown() noexcept(true) { return std::make_shared<PinStyle>(PinStyle(IM_COL32(191,134,90,255), 0, 4.f, 4.67f, 3.7f, 1.f)); }
        /// @brief <BR>Default red style
        static std::shared_ptr<PinStyle> red()   noexcept(true) { return std::make_shared<PinStyle>(PinStyle(IM_COL32(191,90,90,255), 0, 4.f, 4.67f, 3.7f, 1.f)); }
        /// @brief <BR>Default white style
        static std::shared_ptr<PinStyle> white() noexcept(true) { return std::make_shared<PinStyle>(PinStyle(IM_COL32(255,255,255,255), 5, 4.f, 4.67f, 4.2f, 1.f)); }
    };

    // -----------------------------------------------------------------------------------------------------------------
    // NODE'S PROPERTIES

    typedef uintptr_t NodeUID;

    /**
     * @brief Defines the visual appearance of a node
     */
    class NodeStyle
    {
    public:
        using font_t = ImFont*;

        /***/
        NodeStyle() = delete;
        /***/
        constexpr NodeStyle(ImU32 header_bg, ImColor header_title_color, float radius) noexcept(true)
          : header_title_font(nullptr), bg(IM_COL32(55,64,75,255)), header_bg(header_bg), header_title_color(header_title_color), 
            border_color( IM_COL32(30,38,41,140) ), border_selected_color(IM_COL32(170, 190, 205, 230)),
            padding(13.7f, 6.f, 13.7f, 2.f), radius(radius), border_thickness(-1.35f),
            border_selected_thickness(2.f)
        {}
        /// @brief Font used for the title.
        font_t  header_title_font;
        /// @brief Body's background color
        ImU32   bg;
        /// @brief Header's background color
        ImU32   header_bg;
        /// @brief Header title color
        ImColor header_title_color;
        /// @brief Border color
        ImU32   border_color;
        /// @brief Border color when selected
        ImU32   border_selected_color;

        /// @brief Body's content padding (Left Top Right Bottom)
        ImVec4 padding;
        /// @brief Edges rounding
        float radius;
        /// @brief Border thickness
        float border_thickness;
        /// @brief Border thickness when selected
        float border_selected_thickness;
    public:
        /// @brief <BR>Default cyan style
        static std::shared_ptr<NodeStyle> cyan()  noexcept(true) { return std::make_shared<NodeStyle>(IM_COL32(71,142,173,255), ImColor(233,241,244,255), 6.5f); }
        /// @brief <BR>Default green style
        static std::shared_ptr<NodeStyle> green() noexcept(true) { return std::make_shared<NodeStyle>(IM_COL32(90,191,93,255), ImColor(233,241,244,255), 3.5f); }
        /// @brief <BR>Default red style
        static std::shared_ptr<NodeStyle> red()   noexcept(true) { return std::make_shared<NodeStyle>(IM_COL32(191,90,90,255), ImColor(233,241,244,255), 11.f); }
        /// @brief <BR>Default brown style
        static std::shared_ptr<NodeStyle> brown() noexcept(true) { return std::make_shared<NodeStyle>(IM_COL32(191,134,90,255), ImColor(233,241,244,255), 6.5f); }
    };

    // -----------------------------------------------------------------------------------------------------------------
    // LINK

    /**
     * @brief Link between two Pins of two different Nodes
     */
    class Link
    {
    public:
        /**
         * @brief <BR>Construct a link
         * @param left Pointer to the output Pin of the Link
         * @param right Pointer to the input Pin of the Link
         * @param inf Pointer to the Handler that contains the Link
         */
        constexpr explicit Link(Pin* left, Pin* right, ImNodeFlow* inf) noexcept(true)
          : m_left(left), m_right(right), m_inf(inf), m_hovered(false), m_selected(false)
        {}

        /**
         * @brief <BR>Destruction of a link
         * @details Deletes references of this links form connected pins
         */
        ~Link() noexcept(true);

        /**
         * @brief <BR>Looping function to update the Link
         * @details Draws the Link and updates Hovering and Selected status.
         */
        void update() noexcept(true);

        /**
         * @brief <BR>Get Left pin of the link
         * @return Pointer to the Pin
         */
        [[nodiscard]] constexpr Pin* left() const noexcept(true) 
        { return m_left; }

        /**
         * @brief <BR>Get Right pin of the link
         * @return Pointer to the Pin
         */
        [[nodiscard]] constexpr Pin* right() const noexcept(true)
        { return m_right; }

        /**
         * @brief <BR>Get hovering status
         * @return [TRUE] If the link is hovered in the current frame
         */
        [[nodiscard]] constexpr bool isHovered() const noexcept(true)
        { return m_hovered; }

        /**
         * @brief <BR>Get selected status
         * @return [TRUE] If the link is selected in the current frame
         */
        [[nodiscard]] constexpr bool isSelected() const noexcept(true)
        { return m_selected; }

    private:
        Pin*        m_left;
        Pin*        m_right;
        ImNodeFlow* m_inf;
        bool        m_hovered;
        bool        m_selected;
    };

    // -----------------------------------------------------------------------------------------------------------------
    // HANDLER

    /**
     * @brief Grid's the color parameters
     */
    struct InfColors
    {
        /// @brief Background of the grid
        ImU32 background = IM_COL32(33,41,45,255);
        /// @brief Main lines of the grid
        ImU32 grid       = IM_COL32(200, 200, 200, 40);
        /// @brief Secondary lines
        ImU32 subGrid    = IM_COL32(200, 200, 200, 10);
    };

    /**
     * @brief ALl the grid's appearance parameters. Sizes + Colors
     */
    struct InfStyler
    {
        /// @brief Size of main grid
        float     grid_size = 50.f;
        /// @brief Sub-grid divisions for Node snapping
        float     grid_subdivisions = 5.f;
        /// @brief ImNodeFlow colors
        InfColors colors;
    };

    /**
     * @brief Main node editor
     * @details Handles the infinite grid, nodes and links. Also handles all the logic.
     */
    class ImNodeFlow
    {
    private:
        static int m_instances;
    public:
        /**
         * @brief <BR>Instantiate a new editor with default name.
         * <BR> Editor name will be "FlowGrid + the number of editors"
         */
        ImNodeFlow() noexcept(true)
         : ImNodeFlow("FlowGrid" + std::to_string(m_instances)) 
        {}

        /**
         * @brief <BR>Instantiate a new editor with given name
         * @details Creates a new Node Editor with the given name.
         * @param name Name of the editor
         */
        explicit ImNodeFlow( std::string&& name ) noexcept(true)
         : m_name(std::move(name))
        {
            m_instances++;
            m_context.config().extra_window_wrapper = true;
            m_context.config().color = m_style.colors.background;
        }

        /**
         * @brief <BR>Handler loop
         * @details Main update function. Refreshes all the logic and draws everything. Must be called every frame.
         */
        void update() noexcept(true);

        /**
         * @brief <BR>Add a node to the grid
         * @tparam T Derived class of <BaseNode> to be added
         * @tparam Params types of optional args to forward to derived class ctor
         * @param pos Position of the Node in grid coordinates
         * @param args Optional arguments to be forwarded to derived class ctor
         * @return Shared pointer of the pushed type to the newly added node
         *
         * Inheritance is checked at compile time, \<T> MUST be derived from BaseNode.
         */
        template<typename T, typename... Params>
        std::shared_ptr<T> addNode(const ImVec2& pos, Params&&... args) noexcept(true);

    private:
        /**
         * @brief <BR>Helper struct for creating a node struct from a lambda
         * @sa addLambdaNode which wraps creating one of these
         * @tparam L the type of the lambda
         * @tparam B always BaseNode, tparam because BaseNode is incomplete here
         */
        template <typename L, typename B = BaseNode>
        struct NodeWrapper : public B
        {
            L mLambda;
            NodeWrapper(L&& l): BaseNode(), mLambda(std::forward<L>(l)) {}
            void draw() { mLambda(this); }
        };

    public:
        /**
         * @brief <BR>Add a node whos operation can be defined within a lambda.
         * @tparam L the type of the lambda
         * @param lambda the lambda that defines the nodes operation
         * @param pos the position at which to place the node
         */
        template<typename L>
        std::shared_ptr<NodeWrapper<L>> addLambdaNode(L&& lambda, const ImVec2& pos) noexcept(true)
        {
            return addNode<NodeWrapper<L>>(pos, std::forward<L>(lambda));
        }

        /**
         * @brief <BR>Add a node to the grid
         * @tparam T Derived class of <BaseNode> to be added
         * @tparam Params types of optional args to forward to derived class ctor
         * @param pos Position of the Node in screen coordinates
         * @param args Optional arguments to be forwarded to derived class ctor
         * @return Shared pointer of the pushed type to the newly added node
         *
         * Inheritance is checked at compile time, \<T> MUST be derived from BaseNode.
         */
        template<typename T, typename... Params>
        std::shared_ptr<T> placeNodeAt(const ImVec2& pos, Params&&... args) noexcept(true);

        /**
         * @brief <BR>Add a node to the grid using mouse position
         * @tparam T Derived class of <BaseNode> to be added
         * @tparam Params types of optional args to forward to derived class ctor
         * @param args Optional arguments to be forwarded to derived class ctor
         * @return Shared pointer of the pushed type to the newly added node
         *
         * Inheritance is checked at compile time, \<T> MUST be derived from BaseNode.
         */
        template<typename T, typename... Params>
        std::shared_ptr<T> placeNode(Params&&... args) noexcept(true);

        /**
         * @brief <BR>Add link to the handler internal list
         * @param link Reference to the link
         */
        void addLink(std::shared_ptr<Link>& link) noexcept(true);

        /**
         * @brief <BR>Pop-up when link is "dropped"
         * @details Sets the content of a pop-up that can be displayed when dragging a link in the open instead of onto another pin.
         * @details If "key = ImGuiKey_None" the pop-up will always open when a link is dropped.
         * @param content Function or Lambda containing only the contents of the pop-up and the subsequent logic
         * @param key Optional key required in order to open the pop-up
         */
        void droppedLinkPopUpContent(std::function<void(Pin* dragged)> content, ImGuiKey key = ImGuiKey_None) noexcept(true)
        { 
          m_droppedLinkPopUp = std::move(content); 
          m_droppedLinkPupUpComboKey = key; 
        }

        /**
         * @brief <BR>Pop-up when right-clicking
         * @details Sets the content of a pop-up that can be displayed when right-clicking on the grid.
         * @param content Function or Lambda containing only the contents of the pop-up and the subsequent logic
         */
        void rightClickPopUpContent(std::function<void(BaseNode* node)> content) noexcept(true)
        { m_rightClickPopUp = std::move(content); }

        /**
         * @brief <BR>Get mouse clicking status
         * @return [TRUE] if mouse is clicked and click hasn't been consumed
         */
        [[nodiscard]] constexpr bool getSingleUseClick() const noexcept(true) 
        { return m_singleUseClick; }

        /**
         * @brief <BR>Consume the click for the given frame
         */
        constexpr void consumeSingleUseClick() noexcept(true)
        { m_singleUseClick = false; }

        /**
         * @brief <BR>Get editor's name
         * @return Const reference to editor's name
         */
        inline std::string_view getName() const noexcept(true)
        { return m_name; }

        /**
         * @brief <BR>Get editor's position
         * @return Const reference to editor's position in screen coordinates
         */
        inline const ImVec2& getPos() const noexcept(true)
        { return m_context.origin(); }

        /**
         * @brief <BR>Get editor's grid scroll
         * @details Scroll is the offset from the origin of the grid, changes while navigating the grid.
         * @return Const reference to editor's grid scroll
         */
        constexpr const ImVec2& getScroll() noexcept(true)
        { return m_context.scroll(); }

        /**
         * @brief <BR>Get editor's list of nodes
         * @return Const reference to editor's internal nodes list
         */
        constexpr std::unordered_map<NodeUID, std::shared_ptr<BaseNode>>& getNodes() noexcept(true)
        { return m_nodes; }

        /**
         * @brief <BR>Get nodes count
         * @return Number of nodes present in the editor
         */
        inline uint32_t getNodesCount() noexcept(true)
        { return (uint32_t)m_nodes.size(); }

        /**
         * @brief <BR>Get editor's list of links
         * @return Const reference to editor's internal links list
         */
        constexpr const std::vector<std::weak_ptr<Link>>& getLinks() noexcept(true)
        { return m_links; }

        /**
         * @brief <BR>Get zooming viewport
         * @return Reference to editor's internal context for zoom support
         */
        constexpr ContainedContext& getContext() noexcept(true)
        { return m_context; }

        /**
         * @brief <BR>Get dragging status
         * @return [TRUE] if a Node is being dragged around the grid
         */
        [[nodiscard]] constexpr bool isNodeDragged() const noexcept(true)
        { return m_draggingNode; }

        /**
         * @brief <BR>Get current style
         * @return Reference to style variables
         */
        constexpr InfStyler& getStyle() noexcept(true)
        { return m_style; }

        /**
         * @brief <BR>Set editor's size
         * @param size Editor's size. Set to (0, 0) to auto-fit.
         */
        inline void setSize(const ImVec2& size) noexcept(true)
        { m_context.config().size = size; }

        /**
         * @brief <BR>Set dragging status
         * @param state New dragging state
         *
         * The new state will only be updated one at the start of each frame.
         */
        constexpr void draggingNode(bool state) noexcept(true)
        { m_draggingNodeNext = state; }

        /**
         * @brief <BR>Set what pin is being hovered
         * @param hovering Pointer to the hovered pin
         */
        constexpr void hovering(Pin* hovering) noexcept(true)
        { m_hovering = hovering; }

        /**
         * @brief <BR>Set what node is being hovered
         * @param hovering Pointer to the hovered node
         */
        constexpr void hoveredNode(BaseNode* hovering) noexcept(true)
        { m_hoveredNode = hovering; }

        /**
         * @brief <BR>Convert coordinates from screen to grid
         * @param p Point in screen coordinates to be converted
         * @return Point in grid's coordinates
         */
        ImVec2 screen2grid(const ImVec2& p) noexcept(true);

        /**
         * @brief <BR>Convert coordinates from grid to screen
         * @param p Point in grid's coordinates to be converted
         * @return Point in screen coordinates
         */
        ImVec2 grid2screen(const ImVec2 &p) noexcept(true);

        /**
         * @brief <BR>Check if mouse is on selected node
         * @return [TRUE] if the mouse is hovering a selected node
         */
        bool on_selected_node() noexcept(true);

        /**
         * @brief <BR>Check if mouse is on a free point on the grid
         * @return [TRUE] if the mouse is not hovering a node or a link
         */
        bool on_free_space() noexcept(true);

        /**
         * @brief <BR>Get recursion blacklist for nodes
         * @return Reference to blacklist
         */
        constexpr std::vector<std::string>& get_recursion_blacklist() noexcept(true)
        { return m_pinRecursionBlacklist; }
    private:
        const std::string m_name;
        ContainedContext  m_context;

        bool m_singleUseClick = false;

        std::unordered_map<NodeUID, std::shared_ptr<BaseNode>> m_nodes;
        std::vector<std::string> m_pinRecursionBlacklist;
        std::vector<std::weak_ptr<Link>> m_links;

        std::function<void(Pin* dragged)> m_droppedLinkPopUp;
        ImGuiKey m_droppedLinkPupUpComboKey = ImGuiKey_None;
        Pin* m_droppedLinkLeft = nullptr;
        std::function<void(BaseNode* node)> m_rightClickPopUp;
        BaseNode* m_hoveredNodeAux = nullptr;

        BaseNode* m_hoveredNode = nullptr;
        bool      m_draggingNode = false;
        bool      m_draggingNodeNext = false;
        Pin*      m_hovering = nullptr;
        Pin*      m_dragOut = nullptr;

        InfStyler m_style;
    };

    // -----------------------------------------------------------------------------------------------------------------
    // BASE NODE

    /**
     * @brief Parent class for custom nodes
     * @details Main class from which custom nodes can be created. All interactions with the main grid are handled internally.
     */
    class BaseNode
    {
    public:
        /***/
        BaseNode() noexcept(true)
        { m_uid = reinterpret_cast<uintptr_t>(this); }
        /***/
        virtual ~BaseNode() = default;

        /**
         * @brief <BR>Main loop of the node
         * @details Updates position, hovering and selected status, and renders the node. Must be called each frame.
         */
        void update() noexcept(true);

        /**
         * @brief <BR>Content of the node
         * @details Function to be implemented by derived custom nodes.
         *          Must contain the body of the node. If left empty the node will only have input and output pins.
         */
        virtual void draw() noexcept(true) = 0;

        /**
         * @brief <BR>Add an Input to the node
         * @details Will add an Input pin to the node with the given name and data type.
         *          <BR> <BR> In this case the name of the pin will also be its UID.
         *          <BR> <BR> The UID must be unique only in the context of the current node's inputs.
         * @tparam T Type of the data the pin will handle
         * @param name Name of the pin
         * @param defReturn Default return value when the pin is not connected
         * @param filter Connection filter
         * @param style Style of the pin
         * @return Shared pointer to the newly added pin
         */
        template<typename T>
        std::shared_ptr<InPin<T>> addIN(const std::string& name, T defReturn, std::function<bool(Pin*, Pin*)> filter, std::shared_ptr<PinStyle> style = nullptr) noexcept(true);

        /**
         * @brief <BR>Add an Input to the node
         * @details Will add an Input pin to the node with the given name and data type.
         *          <BR> <BR> The UID must be unique only in the context of the current node's inputs.
         * @tparam T Type of the data the pin will handle
         * @tparam U Type of the UID
         * @param uid Unique identifier of the pin
         * @param name Name of the pin
         * @param defReturn Default return value when the pin is not connected
         * @param filter Connection filter
         * @param style Style of the pin
         * @return Shared pointer to the newly added pin
         */
        template<typename T, typename U>
        std::shared_ptr<InPin<T>> addIN_uid(const U& uid, const std::string& name, T defReturn, std::function<bool(Pin*, Pin*)> filter, std::shared_ptr<PinStyle> style = nullptr) noexcept(true);

        /**
         * @brief <BR>Remove input pin
         * @tparam U Type of the UID
         * @param uid Unique identifier of the pin
         */
        template<typename U>
        void dropIN(const U& uid) noexcept(true);

        /**
         * @brief <BR>Remove input pin
         * @param uid Unique identifier of the pin
         */
        void dropIN(const char* uid) noexcept(true);

        /**
         * @brief <BR>Show a temporary input pin
         * @details Will show an input pin with the given name.
         *          The pin is created the first time showIN is called and kept alive as long as showIN is called each frame.
         *          <BR> <BR> In this case the name of the pin will also be its UID.
         *          <BR> <BR> The UID must be unique only in the context of the current node's inputs.
         * @tparam T Type of the data the pin will handle
         * @param name Name of the pin
         * @param defReturn Default return value when the pin is not connected
         * @param filter Connection filter
         * @param style Style of the pin
         * @return Const reference to the value of the connected link for the current frame of defReturn
         */
        template<typename T>
        const T& showIN(const std::string& name, T defReturn, std::function<bool(Pin*, Pin*)> filter, std::shared_ptr<PinStyle> style = nullptr) noexcept(true);

        /**
         * @brief <BR>Show a temporary input pin
         * @details Will show an input pin with the given name and UID.
         *          The pin is created the first time showIN_uid is called and kept alive as long as showIN_uid is called each frame.
         *          <BR> <BR> The UID must be unique only in the context of the current node's inputs.
         * @tparam T Type of the data the pin will handle
         * @tparam U Type of the UID
         * @param uid Unique identifier of the pin
         * @param name Name of the pin
         * @param defReturn Default return value when the pin is not connected
         * @param filter Connection filter
         * @param style Style of the pin
         * @return Const reference to the value of the connected link for the current frame of defReturn
         */
        template<typename T, typename U>
        const T& showIN_uid(const U& uid, const std::string& name, T defReturn, std::function<bool(Pin*, Pin*)> filter, std::shared_ptr<PinStyle> style = nullptr) noexcept(true);

        /**
         * @brief <BR>Add an Output to the node
         * @details Must be called in the node constructor. WIll add an Output pin to the node with the given name and data type.
         *          <BR> <BR> In this case the name of the pin will also be its UID.
         *          <BR> <BR> The UID must be unique only in the context of the current node's outputs.
         * @tparam T Type of the data the pin will handle
         * @param name Name of the pin
         * @param filter Connection filter
         * @param style Style of the pin
         * @return Shared pointer to the newly added pin. Must be used to set the behaviour
         */
        template<typename T>
        [[nodiscard]] std::shared_ptr<OutPin<T>> addOUT(const std::string& name, std::shared_ptr<PinStyle> style = nullptr) noexcept(true);

        /**
         * @brief <BR>Add an Output to the node
         * @details Must be called in the node constructor. WIll add an Output pin to the node with the given name and data type.
         *          <BR> <BR> The UID must be unique only in the context of the current node's outputs.
         * @tparam T Type of the data the pin will handle
         * @tparam U Type of the UID
         * @param uid Unique identifier of the pin
         * @param name Name of the pin
         * @param filter Connection filter
         * @param style Style of the pin
         * @return Shared pointer to the newly added pin. Must be used to set the behaviour
         */
        template<typename T, typename U>
        [[nodiscard]] std::shared_ptr<OutPin<T>> addOUT_uid(const U& uid, const std::string& name, std::shared_ptr<PinStyle> style = nullptr) noexcept(true);

        /**
         * @brief <BR>Remove output pin
         * @tparam U Type of the UID
         * @param uid Unique identifier of the pin
         */
        template<typename U>
        void dropOUT(const U& uid) noexcept(true);

        /**
         * @brief <BR>Remove output pin
         * @param uid Unique identifier of the pin
         */
        void dropOUT(const char* uid) noexcept(true);

        /**
         * @brief <BR>Show a temporary output pin
         * @details Will show an output pin with the given name.
         *          The pin is created the first time showOUT is called and kept alive as long as showOUT is called each frame.
         *          <BR> <BR> In this case the name of the pin will also be its UID.
         *          <BR> <BR> The UID must be unique only in the context of the current node's outputs.
         * @tparam T Type of the data the pin will handle
         * @param name Name of the pin
         * @param behaviour Function or lambda expression used to calculate output value
         * @param filter Connection filter
         * @param style Style of the pin
         */
        template<typename T>
        void showOUT(const std::string& name, std::function<T()> behaviour, std::shared_ptr<PinStyle> style = nullptr) noexcept(true);

        /**
         * @brief <BR>Show a temporary output pin
         * @details Will show an output pin with the given name.
         *          The pin is created the first time showOUT_uid is called and kept alive as long as showOUT_uid is called each frame.
         *          <BR> <BR> The UID must be unique only in the context of the current node's outputs.
         * @tparam T Type of the data the pin will handle
         * @tparam U Type of the UID
         * @param uid Unique identifier of the pin
         * @param name Name of the pin
         * @param behaviour Function or lambda expression used to calculate output value
         * @param filter Connection filter
         * @param style Style of the pin
         */
        template<typename T, typename U>
        void showOUT_uid(const U& uid, const std::string& name, std::function<T()> behaviour, std::shared_ptr<PinStyle> style = nullptr) noexcept(true);

        /**
         * @brief <BR>Get Input value from an InPin
         * @details Get a reference to the value of an input pin, the value is stored in the output pin at the other end of the link.
         * @tparam T Data type
         * @tparam U Type of the UID
         * @param uid Unique identifier of the pin
         * @return Const reference to the value
         */
        template<typename T, typename U>
        const T& getInVal(const U& uid) noexcept(true);

        /**
         * @brief <BR>Get Input value from an InPin
         * @details Get a reference to the value of an input pin, the value is stored in the output pin at the other end of the link.
         * @tparam T Data type
         * @param uid Unique identifier of the pin
         * @return Const reference to the value
         */
        template<typename T>
        const T& getInVal(const char* uid) noexcept(true);

        /**
         * @brief <BR>Get generic reference to input pin
         * @tparam U Type of the UID
         * @param uid Unique identifier of the pin
         * @return Generic pointer to the pin
         */
        template<typename U>
        Pin* inPin(const U& uid) noexcept(true);

        /**
         * @brief <BR>Get generic reference to input pin
         * @param uid Unique identifier of the pin
         * @return Generic pointer to the pin
         */
        Pin* inPin(const char* uid) noexcept(true);

        /**
         * @brief <BR>Get generic reference to output pin
         * @tparam U Type of the UID
         * @param uid Unique identifier of the pin
         * @return Generic pointer to the pin
         */
        template<typename U>
        Pin* outPin(const U& uid) noexcept(true);

        /**
         * @brief <BR>Get generic reference to output pin
         * @param uid Unique identifier of the pin
         * @return Generic pointer to the pin
         */
        Pin* outPin(const char* uid) noexcept(true);

        /**
         * @brief <BR>Get internal input pins list
         * @return Const reference to node's internal list
         */
        const std::vector<std::shared_ptr<Pin>>& getIns() noexcept(true)
        { return m_ins; }

        /**
         * @brief <BR>Get internal output pins list
         * @return Const reference to node's internal list
         */
        const std::vector<std::shared_ptr<Pin>>& getOuts() noexcept(true)
        { return m_outs; }

        /**
         * @brief <BR>Delete itself
         */
        constexpr void destroy() noexcept(true)
        { m_destroyed = true; }

        /*
         * @brief <BR>Get if node must be deleted
         */
        [[nodiscard]] constexpr bool toDestroy() const noexcept(true)
        { return m_destroyed; }

        /**
         * @brief <BR>Get hovered status
         * @return [TRUE] if the mouse is hovering the node
         */
        bool isHovered() noexcept(true);

        /**
         * @brief <BR>Get node's UID
         * @return Node's unique identifier
         */
        [[nodiscard]] constexpr NodeUID getUID() const noexcept(true)
        { return m_uid; }

        /**
         * @brief <BR>Get node name
         * @return Const reference to the node's name
         */
        inline std::string_view getName() const noexcept(true)
        { return m_title; }

        /**
         * @brief <BR>Get node size
         * @return Const reference to the node's size
         */
        constexpr const ImVec2& getSize() const noexcept(true)
        { return  m_size; }

        /**
         * @brief <BR>Get node position
         * @return Const reference to the node's position
         */
        constexpr const ImVec2& getPos() const noexcept(true)
        { return  m_pos; }

        /**
         * @brief <BR>Get grid handler bound to node
         * @return Pointer to the handler
         */
        constexpr ImNodeFlow* getHandler() noexcept(true)
        { return m_inf; }

        /**
         * @brief <BR>Get node's style
         * @return Shared pointer to the node's style
         */
        constexpr const std::shared_ptr<NodeStyle>& getStyle() noexcept(true)
        { return m_style; }

        /**
         * @brief <BR>Get selected status
         * @return [TRUE] if the node is selected
         */
        [[nodiscard]] constexpr bool isSelected() const noexcept(true)
        { return m_selected; }

        /**
         * @brief <BR>Get dragged status
         * @return [TRUE] if the node is being dragged
         */
        [[nodiscard]] constexpr bool isDragged() const noexcept(true)
        { return m_dragged; }

        /**
         * @brief <BR>Set node's uid
         * @param uid Node's unique identifier
         */
        constexpr BaseNode* setUID(NodeUID uid) noexcept(true)
        { m_uid = uid; return this; }

        /**
         * @brief <BR>Set node's name
         * @param name New title
         */
        inline BaseNode* setTitle(const std::string& title) noexcept(true)
        { m_title = title; return this; }

        /**
         * @brief <BR>Set node's position
         * @param pos Position in grid coordinates
         */
        constexpr BaseNode* setPos(const ImVec2& pos) noexcept(true)
        { m_pos = pos; m_posTarget = pos; return this; }

        /**
         * @brief <BR>Set ImNodeFlow handler
         * @param inf Grid handler for the node
         */
        constexpr BaseNode* setHandler(ImNodeFlow* inf) noexcept(true)
        { m_inf = inf; return this; }

        /**
         * @brief Set node's style
         * @param style New style
         */
        BaseNode* setStyle(std::shared_ptr<NodeStyle> style) noexcept(true)
        { m_style = std::move(style); return this; }

        /**
         * @brief <BR>Set selected status
         * @param state New selected state
         *
         * Status only updates when updatePublicStatus() is called
         */
        BaseNode* selected(bool state) { m_selectedNext = state; return this; }

        /**
         * @brief <BR>Update the isSelected status of the node
         */
        void updatePublicStatus() { m_selected = m_selectedNext; }
    private:
        NodeUID m_uid = 0;
        std::string m_title;
        ImVec2 m_pos, m_posTarget;
        ImVec2 m_size;
        ImNodeFlow* m_inf = nullptr;
        std::shared_ptr<NodeStyle> m_style;
        bool m_selected = false, m_selectedNext = false;
        bool m_dragged = false;
        bool m_destroyed = false;

        std::vector<std::shared_ptr<Pin>> m_ins;
        std::vector<std::pair<int, std::shared_ptr<Pin>>> m_dynamicIns;
        std::vector<std::shared_ptr<Pin>> m_outs;
        std::vector<std::pair<int, std::shared_ptr<Pin>>> m_dynamicOuts;
    };

    // -----------------------------------------------------------------------------------------------------------------
    // PINS

    /**
     * @brief Pins type identifier
     */
    enum PinType
    {
        PinType_Input,
        PinType_Output
    };

    /**
     * @brief Generic base class for pins
     */
    class Pin
    {
    public:
        /***/
        Pin() = delete;
        /**
         * @brief <BR>Generic pin constructor
         * @param name Name of the pin
         * @param filter Connection filter
         * @param kind Specifies Input or Output
         * @param parent Pointer to the Node containing the pin
         * @param inf Pointer to the Grid Handler the pin is in (same as parent)
         * @param style Style of the pin
         */
        explicit Pin(PinUID uid, std::string name, std::shared_ptr<PinStyle> style, PinType kind, BaseNode* parent, ImNodeFlow** inf) noexcept(true)
          : m_uid(uid), m_name(std::move(name)), m_style(std::move(style)), 
            m_pos(0.f, 0.f), m_size(0.f, 0.f), m_type(kind), m_parent(parent), 
            m_inf(inf), m_renderer(nullptr)
        {
            if(!m_style)
                m_style = PinStyle::cyan();
        }
        
        /***/
        virtual ~Pin() = default;

        /**
         * @brief <BR>Main loop of the pin
         * @details Updates position, hovering and dragging status, and renders the pin. Must be called each frame.
         */
        void update() noexcept(true);

        /**
         * @brief <BR>Draw default pin's socket
         */
        void drawSocket() noexcept(true);

        /**
         * @brief <BR>Draw default pin's decoration (border, bg, and hover overlay)
         */
        void drawDecoration() noexcept(true);

        /**
         * @brief <BR>Used by output pins to calculate their values
         */
        virtual void resolve() noexcept(true) = 0;

        /**
         * @brief <BR>Custom render function to override Pin appearance
         * @param r Function or lambda expression with new ImGui rendering
         */
        Pin* renderer(std::function<void(Pin* p)> r) noexcept(true)
        { m_renderer = std::move(r); return this; }

        /**
         * @brief <BR>Create link between pins
         * @param other Pointer to the other pin
         */
        virtual void createLink(Pin* other) noexcept(true) = 0;

        /**
         * @brief <BR>Set the reference to a link
         * @param link Smart pointer to the link
         */
        virtual void setLink( [[maybe_unused]] std::shared_ptr<Link>& link) noexcept(true) = 0;

        /**
         * @brief <BR>Delete link reference
         */
        virtual void deleteLink() noexcept(true) = 0;

        /**
         * @brief <BR>Get connected status
         * @return [TRUE] if the pin is connected
         */
        virtual bool isConnected() noexcept(true) = 0;

        /**
         * @brief <BR>Get pin's link
         * @return Weak_ptr reference to pin's link
         */
        virtual std::weak_ptr<Link> getLink() noexcept(true)
        { return std::weak_ptr<Link>{}; }

        /**
         * @brief <BR>Get pin's UID
         * @return Unique identifier of the pin
         */
        [[nodiscard]] constexpr PinUID getUid() const noexcept(true)
        { return m_uid; }

        /**
         * @brief <BR>Get pin's name
         * @return Const reference to pin's name
         */
        inline std::string_view getName() const noexcept(true)
        { return m_name; }

        /**
         * @brief <BR>Get pin's position
         * @return Const reference to pin's position in grid coordinates
         */
        [[nodiscard]] constexpr const ImVec2& getPos() const noexcept(true)
        { return m_pos; }

        /**
         * @brief <BR>Get pin's hit-box size
         * @return Const reference to pin's hit-box size
         */
        [[nodiscard]] constexpr const ImVec2& getSize() const noexcept(true)
        { return m_size; }

        /**
         * @brief <BR>Get pin's parent node
         * @return Generic type pointer to pin's parent node. (Node that contains it)
         */
        BaseNode* getParent() noexcept(true)
        { return m_parent; }

        /**
         * @brief <BR>Get pin's type
         * @return The pin type. Either Input or Output
         */
        PinType getType() noexcept(true) { return m_type; }

        /**
         * @brief <BR>Get pin's data type (aka: \<T>)
         * @return String containing unique information identifying the data type
         */
        [[nodiscard]] virtual const std::type_info& getDataType() const = 0;

        /**
         * @brief <BR>Get pin's style
         * @return Smart pointer to pin's style
         */
        std::shared_ptr<PinStyle>& getStyle() { return m_style; }

        /**
         * @brief <BR>Get pin's link attachment point (socket)
         * @return Grid coordinates to the attachment point between the link and the pin's socket
         */
        virtual ImVec2 pinPoint() = 0;

        /**
         * @brief <BR>Calculate pin's width pre-rendering
         * @return The with of the pin once it will be rendered
         */
        float calcWidth() { return ImGui::CalcTextSize(m_name.c_str()).x; }

        /**
         * @brief <BR>Set pin's position
         * @param pos Position in screen coordinates
         */
        void setPos(ImVec2 pos) { m_pos = pos; }
    protected:
        PinUID                      m_uid;
        std::string                 m_name;
        std::shared_ptr<PinStyle>   m_style;
        ImVec2                      m_pos;
        ImVec2                      m_size;
        PinType                     m_type;
        BaseNode*                   m_parent;
        ImNodeFlow**                m_inf;
        std::function<void(Pin* p)> m_renderer;
    };

    /**
     * @brief Collection of Pin's collection filters
     */
    class ConnectionFilter
    {
    public:
        static std::function<bool(Pin*, Pin*)> None()     noexcept(true) { return []( [[maybe_unused]] Pin* out, [[maybe_unused]] Pin* in) noexcept(true) { return true; }; }
        static std::function<bool(Pin*, Pin*)> SameType() noexcept(true) { return []( [[maybe_unused]] Pin* out, [[maybe_unused]] Pin* in) noexcept(true) { return out->getDataType() == in->getDataType(); }; }
        static std::function<bool(Pin*, Pin*)> Numbers()  noexcept(true) { return []( [[maybe_unused]] Pin* out, [[maybe_unused]] Pin* in) noexcept(true) { return out->getDataType() == typeid(double) || out->getDataType() == typeid(float) || out->getDataType() == typeid(int); }; }
    };

    /**
     * @brief Input specific pin
     * @details Derived from the generic class Pin. The input pin owns the link pointer.
     * @tparam T Data type handled by the pin
     */
    template<class T> class InPin : public Pin
    {
    public:
        /**
         * @brief <BR>Input pin constructor
         * @param name Name of the pin
         * @param filter Connection filter
         * @param parent Pointer to the Node containing the pin
         * @param defReturn Default return value when the pin is not connected
         * @param inf Pointer to the Grid Handler the pin is in (same as parent)
         * @param style Style of the pin
         */
        explicit InPin(PinUID uid, const std::string& name, T defReturn, std::function<bool(Pin*, Pin*)> filter, std::shared_ptr<PinStyle> style, BaseNode* parent, ImNodeFlow** inf) noexcept(true)
            : Pin(uid, name, style, PinType_Input, parent, inf), m_emptyVal(defReturn), m_filter(std::move(filter)) {}

        /**
         * @brief <BR>Create link between pins
         * @param other Pointer to the other pin
         */
        void createLink(Pin* other) noexcept(true) override;

        /**
        * @brief <BR>Delete the link connected to the pin
        */
        void deleteLink() noexcept(true) override 
        { m_link.reset(); }

        /**
         * @brief Specify if connections from an output on the same node are allowed
         * @param state New state of the flag
         */
        void allowSameNodeConnections(bool state) noexcept(true) { m_allowSelfConnection = state; }

        /**
         * @brief <BR>Get connected status
         * @return [TRUE] is pin is connected to a link
         */
        bool isConnected() noexcept(true) override
        { return m_link != nullptr; }

        /**
         * @brief <BR>Get pin's link
         * @return Weak_ptr reference to the link connected to the pin
         */
        std::weak_ptr<Link> getLink() noexcept(true) override 
        { return m_link; }

        /**
         * @brief <BR>Get InPin's connection filter
         * @return InPin's connection filter configuration
         */
        [[nodiscard]] const std::function<bool(Pin*, Pin*)>& getFilter() const noexcept(true)
        { return m_filter; }

        /**
         * @brief <BR>Get pin's data type (aka: \<T>)
         * @return String containing unique information identifying the data type
         */
        [[nodiscard]] const std::type_info& getDataType() const noexcept(true) override 
        { return typeid(T); };

        /**
         * @brief <BR>Get pin's link attachment point (socket)
         * @return Grid coordinates to the attachment point between the link and the pin's socket
         */
        ImVec2 pinPoint() noexcept(true) override 
        { return m_pos + ImVec2(-m_style->extra.socket_padding, m_size.y / 2); }

        /**
         * @brief <BR>Get value carried by the connected link
         * @return Reference to the value of the connected OutPin. Or the default value if not connected
         */
        const T& val() noexcept(true);

    protected:
        /**
         * @brief <BR>Used by output pins to calculate their values
         */
        virtual void resolve() noexcept(true) override
        {}

        /**
         * @brief <BR>Add a connected link to the internal list
         * @param link Pointer to the link
         */
        void setLink(std::shared_ptr<Link>&)  noexcept(true) override
        {}

    private:
        std::shared_ptr<Link> m_link;
        T m_emptyVal;
        std::function<bool(Pin*, Pin*)> m_filter;
        bool m_allowSelfConnection = false;
    };

    /**
     * @brief Output specific pin
     * @details Derived from the generic class Pin. The output pin handles the logic.
     * @tparam T Data type handled by the pin
     */
    template<class T> class OutPin : public Pin
    {
    public:
        /**
         * @brief <BR>Output pin constructor
         * @param name Name of the pin
         * @param filter Connection filter
         * @param parent Pointer to the Node containing the pin
         * @param inf Pointer to the Grid Handler the pin is in (same as parent)
         * @param style Style of the pin
         */
        explicit OutPin(PinUID uid, const std::string& name, std::shared_ptr<PinStyle> style, BaseNode* parent, ImNodeFlow** inf)
            :Pin(uid, name, style, PinType_Output, parent, inf)
        {}

        /**
         * @brief <BR>When parent gets deleted, remove the links
         */
        ~OutPin() override {
            std::vector<std::weak_ptr<Link>> links = std::move(m_links);
            for (auto &l: links) if (!l.expired()) l.lock()->right()->deleteLink();
        }

        /**
         * @brief <BR>Create link between pins
         * @param other Pointer to the other pin
         */
        void createLink(Pin* other)  noexcept(true) override;

        /**
         * @brief <BR>Add a connected link to the internal list
         * @param link Pointer to the link
         */
        void setLink(std::shared_ptr<Link>& link)  noexcept(true) override;

        /**
         * @brief <BR>Delete any expired weak pointers to a (now deleted) link
         */
        void deleteLink() noexcept(true) override;

        /**
         * @brief <BR>Get connected status
         * @return [TRUE] is pin is connected to one or more links
         */
        bool isConnected()  noexcept(true) override 
        { return !m_links.empty(); }

        /**
         * @brief <BR>Get pin's link attachment point (socket)
         * @return Grid coordinates to the attachment point between the link and the pin's socket
         */
        ImVec2 pinPoint()  noexcept(true) override 
        { return m_pos + ImVec2(m_size.x + m_style->extra.socket_padding, m_size.y / 2); }

        /**
         * @brief <BR>Get output value
         * @return Const reference to the internal value of the pin
         */
        const T& val() noexcept(true);

        /**
         * @brief <BR>Set logic to calculate output value
         * @details Used to define the pin behaviour. This is what gets the data from the parent's inputs, and applies the needed logic.
         * @param func Function or lambda expression used to calculate output value
         */
        OutPin<T>* behaviour(std::function<T()> func) { m_behaviour = std::move(func); return this; }

        /**
         * @brief <BR>Get pin's data type (aka: \<T>)
         * @return String containing unique information identifying the data type
         */
        [[nodiscard]] const std::type_info& getDataType() const noexcept(true) override 
        { return typeid(T); };

    protected:
        /**
         * @brief <BR>Used by output pins to calculate their values
         */
        virtual void resolve() noexcept(true) override
        {}

    private:
        std::vector<std::weak_ptr<Link>> m_links;
        std::function<T()>               m_behaviour;
        T                                m_val;
    };
}

#include "../src/ImNodeFlow.inl"

#endif
