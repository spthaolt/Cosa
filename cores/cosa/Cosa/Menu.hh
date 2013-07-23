/**
 * @file Cosa/Menu.hh
 * @version 1.0
 *
 * @section License
 * Copyright (C) 2013, Mikael Patel
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 *
 * This file is part of the Arduino Che Cosa project.
 */

#ifndef __COSA_MENU_HH__
#define __COSA_MENU_HH__

#include "Cosa/IOStream.hh"
#include "Cosa/LCD.hh"
#include "Cosa/Keypad.hh"

/**
 * LCD Menu abstraction. Allows definition of menus with sub-menus,
 * items, enumerations, bitsets, range values and actions.
 */
class Menu {
public:
  // Menu type tag code
  enum type_t {
    ITEM,			// Menu item/symbol
    ITEM_LIST,			// Menu item/enum list
    ENUM,			// Menu enumeration variable (one-of)
    BITSET,			// Menu bitset variable (zero-or-many)
    RANGE,			// Menu integer range variable
    ACTION			// Menu action
  } __attribute__((packed));

  // Pointer type for character string in program memory
  typedef const PROGMEM char* str_P;

  // Menu item header. Also used for enumeration symbols
  struct item_t {
    type_t type;		// Item type tag(ITEM)
    str_P name;			// Item string in program memory
  };
  typedef const PROGMEM item_t* item_P;
  typedef const PROGMEM item_P* item_vec_P;

  // Menu item lists
  struct item_list_t {
    item_t item;		// Item header(ITEM_LIST)
    item_vec_P list;		// Item list in program memory
  };
  typedef const PROGMEM item_list_t* item_list_P;

  // Enumeration variable symbols list (one-of)
  struct enum_t {
    item_t item;		// Item header(ENUM)
    item_vec_P list;		// Enum item list in program memory
    uint16_t* value;		// Pointer to value
  };
  typedef const PROGMEM enum_t* enum_P;

  // Bitset variable symbols list (zero-or-many), Item header(BITSET)
  typedef enum_t bitset_t;
  typedef const PROGMEM bitset_t* bitset_P;

  // Integer range variable 
  struct range_t {
    item_t item;		// Item header(RANGE)
    int16_t low;		// Range low value
    int16_t high;		// Range high value
    int16_t* value;		// Pointer to value
  };
  typedef const PROGMEM range_t* range_P;

  /**
   * Menu Action handler. Must be sub-classed and the virtual member
   * function run() must be implemented. Holds the state for the 
   * menu action.
   */
  class Action {
  public:
    /**
     * @override
     * Menu action function for given menu item. Return true(1) if the
     * menu walker should render the display otherwise if the false(0).
     * @param[in] item menu item reference.
     * @return bool
     */
    virtual bool run(Menu::item_P item) = 0;
  };

  // Menu action item
  struct action_t {
    item_t item;		// Item header(ACTION)
    Action* obj;		// Pointer to action handler
  };
  typedef const PROGMEM action_t* action_P;

  /**
   * The Menu Walker reacts to key events from the key pad. It maintains
   * a stack with the path to the current position in the menu three.
   */
  class Walker {
  private:
    // Stack for menu walker path
    static const uint8_t STACK_MAX = 8;
    Menu::item_list_P m_stack[STACK_MAX];
    uint8_t m_top;

    // Current menu list item index
    uint8_t m_ix;

    // Current menu bitset index
    uint8_t m_bv;

    // Item selection state
    bool m_selected;

    // Output stream for menu printout
    IOStream m_out;

  public:
    // Menu walker key index (same as LCDkeypad map for simplicity)
    enum {
      NO_KEY = 0,
      SELECT_KEY,
      LEFT_KEY,
      DOWN_KEY,
      UP_KEY,
      RIGHT_KEY
    } __attribute__((packed));
    
    /** 
     * Construct a menu walker for the given menu. 
     * @param[in] lcd device. 
     * @param[in] root menu item list.
     */
    Walker(LCD::Device* lcd, item_list_P root) : 
      m_top(0),
      m_ix(0),
      m_bv(0),
      m_selected(false),
      m_out(lcd)
    {
      m_stack[m_top] = root;
    }

    /**
     * Print initial menu state.
     */
    void begin() { m_out << clear << *this; }

    /**
     * The menu walker key interpretor. Should be called by a menu
     * controller, i.e. an adapter of controller events to the menu
     * walker key.
     * @param[in] nr key number (index in map).
     */
    void on_key_down(uint8_t nr);

    /**
     * Print menu walker state, current menu position to given
     * output stream.
     * @param[in] outs output stream.
     * @param[in] walker menu walker.
     * @return output stream
     */
    friend IOStream& operator<<(IOStream& outs, Walker& walker);
  };

  /**
   * Menu walker controller for the LCD keypad. Adapt the keypad
   * key down events to the Menu walker. For simplicity the key
   * map for the walker and the LCD keypad are the same.
   */
  class KeypadController : public LCDKeypad {
  public:
    Walker* m_walker;
    
  public:
    /**
     * Construct keypad event adapter for menu walker.
     * @param[in] walker to control.
     */
    KeypadController(Walker* walker) :
      LCDKeypad(),
      m_walker(walker)
    {}
    
    /**
     * @override
     * The menu walker key interpretor.
     * @param[in] nr key number (index in map).
     */
    virtual void on_key_down(uint8_t nr)
    {
      m_walker->on_key_down(nr);
    }
  };
};

/**
 * Support macro to start the definition of a menu in program memory.
 * Used in the form:
 *   MENU_BEGIN(var,name)
 *     MENU_ITEM(item-1)
 *     ...
 *     MENU_ITEM(item-n)
 *   MENU_END(var)
 * @param[in] var menu to create.
 * @param[in] name string for menu.
 */
#define MENU_BEGIN(var,name)				\
  const char var ## _name[] PROGMEM = name;		\
  const Menu::item_P var ## _list[] PROGMEM = {  

/**
 * Support macro to add a menu item in program memory.
 * The item can be any of the menu item types; ITEM, ITEM_LIST,
 * ENUM, RANGE and ACTION.
 * @param[in] var item reference to add.
 */
#define MENU_ITEM(var)					\
  &var.item,

/**
 * Support macro to complete a menu in program memory.
 * @param[in] var menu to create.
 */
#define MENU_END(var)					\
    0							\
  };							\
  const Menu::item_list_t var PROGMEM = {		\
  {							\
    Menu::ITEM_LIST,					\
    var ## _name					\
  },							\
  var ## _list						\
};

/**
 * Support macro to define a menu symbol in program memory.
 * @param[in] var menu symbol to create.
 * @param[in] name string for menu symbol.
 */
#define MENU_SYMB(var,name)				\
  const char var ## _name[] PROGMEM = name;		\
  const Menu::item_t var PROGMEM = {			\
    Menu::ITEM,						\
    var ## _name					\
  };

/**
 * Support macro to start the definition of an enumeration type
 * in program memory. 
 * Used in the form:
 *   MENU_SYMB(symb-1)
 *   ...
 *   MENU_SYMB(symb-n)
 *   MENU_ENUM_BEGIN(var,name)
 *     MENU_ENUM_ITEM(symb-1)
 *     ...
 *     MENU_ENUM_ITEM(symb-n)
 *   MENU_ENUM_END(var)
 * @param[in] var menu enumeration variable to create.
 */
#define MENU_ENUM_BEGIN(var)				\
  const Menu::item_P var ## _list[] PROGMEM = {  

/**
 * Support macro to add an menu item/symbol to an enumeration type
 * in program memory.
 * @param[in] var menu item/symbol.
 */
#define MENU_ENUM_ITEM(var)				\
  &var,

/**
 * Support macro to complete an enumeration type in program memory.
 * @param[in] var menu to create.
 */
#define MENU_ENUM_END(var)				\
  0							\
  };

/**
 * Support macro to define a menu enumeration variable.
 * @param[in] type enumeration list.
 * @param[in] var menu enumeration variable to create.
 * @param[in] name string for menu enumeration variable.
 * @param[in] value variable with runtime value.
 */
#define MENU_ENUM(type,var,name,value)			\
  const char var ## _name[] PROGMEM = name;		\
  const Menu::enum_t var PROGMEM = {			\
  {							\
    Menu::ENUM,						\
    var ## _name					\
  },							\
  type ## _list,					\
  &value						\
};

/**
 * Support macro to define a menu bitset variable.
 * @param[in] type enumeration list.
 * @param[in] var menu bitset variable to create.
 * @param[in] name string for menu bitset variable.
 * @param[in] value variable with runtime value.
 */
#define MENU_BITSET(type,var,name,value)		\
  const char var ## _name[] PROGMEM = name;		\
  const Menu::enum_t var PROGMEM = {			\
  {							\
    Menu::BITSET,					\
    var ## _name					\
  },							\
  type ## _list,					\
  &value						\
};

/**
 * Support macro to define a menu integer range in program memory.
 * @param[in] var menu range item to create.
 * @param[in] name string of menu item.
 * @param[in] low range value.
 * @param[in] high range value.
 * @param[in] value variable name.
 */
#define MENU_RANGE(var,name,low,high,value)		\
  const char var ## _name[] PROGMEM = name;		\
  const Menu::range_t var PROGMEM = {			\
  {							\
    Menu::RANGE,					\
    var ## _name					\
  },							\
  low,							\
  high,							\
  &value						\
};

/**
 * Support macro to define a menu action in program memory.
 * @param[in] var menu action item to create.
 * @param[in] name string of menu item.
 * @param[in] obj pointer to menu action handler.
 */
#define MENU_ACTION(var,name,obj)			\
  const char var ## _name[] PROGMEM = name;		\
  const Menu::action_t var PROGMEM = {			\
  {							\
    Menu::ACTION,					\
    var ## _name					\
  },							\
  &obj							\
  };
#endif