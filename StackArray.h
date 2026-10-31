/*
 *  StackArray.h
 *
 *  Library implementing a generic, dynamic stack (array version).
 *
 *  ---
 *
 *  Copyright (C) 2010  Efstathios Chatzikyriakidis (contact@efxa.org)
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 *  ---
 *
 *  Version 1.0
 *
 *    2010-09-25  Efstathios Chatzikyriakidis  <contact@efxa.org>
 *
 *      - added resize(): for growing, shrinking the array size.
 *
 *    2010-09-23  Efstathios Chatzikyriakidis  <contact@efxa.org>
 *
 *      - added exit(), blink(): error reporting and handling methods.
 *
 *    2010-09-20  Alexander Brevig  <alexanderbrevig@gmail.com>
 *
 *      - added setPrinter(): indirectly reference a Serial object.
 *
 *    2010-09-15  Efstathios Chatzikyriakidis  <contact@efxa.org>
 *
 *      - initial release of the library.
 *
 *  ---
 *
 *  For the latest version see: http://www.arduino.cc/
 */

// header defining the interface of the source.
#ifndef _STACKARRAY_H
#define _STACKARRAY_H

// include Arduino basic header.
#include <Arduino.h>
#include <limits.h>
#include <stdlib.h>

// the definition of the stack class.
template<typename T>
class StackArray {
  public:
    // init the stack (constructor).
    StackArray ();
    
    StackArray(int size);

    // clear the stack (destructor).
    ~StackArray ();

    // push an item to the stack.
    // False leaves previous storage/items/count intact on allocation failure.
    bool push (const T i);
    bool remove(const T item);
    bool contains(const T item) const;
    StackArray(const StackArray&) = delete;
    StackArray& operator=(const StackArray&) = delete;

    // pop an item from the stack.
    T pop ();

    // get an item from the stack.
    T peek () const;
    
    T operator[](int i) const;

    // check if the stack is empty.
    bool isEmpty () const;

    // get the number of items in the stack.
    int count () const;

    // check if the stack is full.
    bool isFull () const;

    // set the printer of the stack.
    void setPrinter (Print & p);

  private:
    // resize the size of the stack.
    bool resize (const int s);

    // exit report method in case of error.
    void exit (const char * m) const;

    // led blinking method in case of error.
    void blink () const;

    // the initial size of the stack.
    static const int initialSize = 2;

    // the pin number of the on-board led.
    static const int ledPin = 13;

    Print * printer; // the printer of the stack.
    T * contents;    // the array of the stack.
    int size;        // the size of the stack.
    int top;         // the top index of the stack.
};

// init the stack (constructor).
template<typename T>
StackArray<T>::StackArray () {
  size = 0;       // set the size of stack to zero.
  top = 0;        // set the initial top index of the stack.
  printer = NULL; // set the printer of stack to point nowhere.

  // allocate enough memory for the array.
  contents = (T *) malloc (sizeof (T) * initialSize);

  // if there is a memory allocation error.
  size = contents ? initialSize : 0;
}


// init the stack (constructor).
template<typename T>
StackArray<T>::StackArray (int s) {
  size = 0;       // set the size of stack to zero.
  top = 0;        // set the initial top index of the stack.
  printer = NULL; // set the printer of stack to point nowhere.

  // allocate enough memory for the array.
  contents = (s > 0 && static_cast<size_t>(s) <= static_cast<size_t>(-1) / sizeof(T))
      ? (T *) malloc (sizeof (T) * s) : NULL;

  // if there is a memory allocation error.
  size = contents ? s : 0;
}



// clear the stack (destructor).
template<typename T>
StackArray<T>::~StackArray () {
  free (contents); // deallocate the array of the stack.

  contents = NULL; // set stack's array pointer to nowhere.
  printer = NULL;  // set the printer of stack to point nowhere.
  size = 0;        // set the size of stack to zero.
  top = 0;         // set the initial top index of the stack.
}

// resize the size of the stack.
template<typename T>
bool StackArray<T>::resize (const int s) {
  if (s <= 0 || s < top || static_cast<size_t>(s) > static_cast<size_t>(-1) / sizeof(T)) return false;
  T* replacement = (T*) realloc(contents, sizeof(T) * s);
  if (!replacement) return false;
  contents = replacement;
  size = s;
  return true;
}

// push an item to the stack.
template<typename T>
bool StackArray<T>::push (const T i) {
  // check if the stack is full.
  if (isFull ()) {
    if (size > INT_MAX / 2) return false;
    if (!resize(size ? size * 2 : initialSize)) return false;
  }

  // store the item to the array.
  contents[top++] = i;
  return true;
}

// pop an item from the stack.
template<typename T>
T StackArray<T>::pop () {
  // check if the stack is empty.
  if (isEmpty ()) return T();

  // fetch the top item from the array.
  T item = contents[--top];

  // shrink size of array if necessary.
  /*if (!isEmpty () && (top <= size / 4))
    resize (size / 2);
*/
  // return the top item from the array.
  return item;
}

// get an item from the stack.
template<typename T>
T StackArray<T>::peek () const {
  // check if the stack is empty.
  if (isEmpty ()) return T();

  // get the top item from the array.
  return contents[top - 1];
}


template<typename T>
T StackArray<T>::operator[](int index) const {
	return (index >= 0 && index < top) ? contents[index] : T();
}


// check if the stack is empty.
template<typename T>
bool StackArray<T>::isEmpty () const {
  return top == 0;
}

// check if the stack is full.
template<typename T>
bool StackArray<T>::isFull () const {
  return top == size;
}

// get the number of items in the stack.
template<typename T>
int StackArray<T>::count () const {
  return top;
}

// set the printer of the stack.
template<typename T>
void StackArray<T>::setPrinter (Print & p) {
  printer = &p;
}

// exit report method in case of error.
template<typename T>
void StackArray<T>::exit (const char * m) const {
  // print the message if there is a printer.
  if (printer)
    printer->println (m);

  // loop blinking until hardware reset.
  blink ();
}

// led blinking method in case of error.
template<typename T>
void StackArray<T>::blink () const {
  // set led pin as output.
  pinMode (ledPin, OUTPUT);

  // continue looping until hardware reset.
  while (true) {
    digitalWrite (ledPin, HIGH); // sets the LED on.
    delay (250);                 // pauses 1/4 of second.
    digitalWrite (ledPin, LOW);  // sets the LED off.
    delay (250);                 // pauses 1/4 of second.
  }

  // solution selected due to lack of exit() and assert().
}

template<typename T>
bool StackArray<T>::contains(const T item) const {
  for (int i = 0; i < top; ++i) if (contents[i] == item) return true;
  return false;
}

template<typename T>
bool StackArray<T>::remove(const T item) {
  for (int i = 0; i < top; ++i) {
    if (contents[i] == item) {
      for (int j = i + 1; j < top; ++j) contents[j - 1] = contents[j];
      --top;
      return true;
    }
  }
  return false;
}

#endif // _STACKARRAY_H
