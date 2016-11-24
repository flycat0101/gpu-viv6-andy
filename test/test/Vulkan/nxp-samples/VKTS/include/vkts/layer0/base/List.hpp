/**
 * VKTS - VulKan ToolS.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) since 2014 Norbert Nopper
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef VKTS_LIST_HPP_
#define VKTS_LIST_HPP_

#include <vkts/vkts.hpp>

#define VKTS_INITIAL_VECTOR_COUNT   16

namespace vkts
{

template<class V>
class ListElement
{

public:

    V value;

    ListElement* prev;
    ListElement* next;

    ListElement(const V& value) :
        value(value), prev(nullptr), next(nullptr)
    {
    }

    ~ListElement()
    {
        prev = nullptr;
        next = nullptr;
    }

};

template<class V>
class List
{

private:

    ListElement<V>* front;
    ListElement<V>* back;

    size_t listSize;

public:

    List() :
        front(nullptr), back(nullptr), listSize(0)
    {
    }

    List(const List& other) :
        front(nullptr), back(nullptr), listSize(0)
    {
        auto walker = other.front;

        while (walker)
        {
            if (!emplaceBack(walker->element))
            {
                clear();

                return;
            }

            walker = walker->next;
        }
    }

    List(List&& other) :
        front(other.front), back(other.back), listSize(other.listSize)
    {
        other.front = nullptr;
        other.back = nullptr;
        other.listSize = 0;
    }

    List& operator= (const List& other)
    {
    	clear();

    	//

        auto walker = other.front;

        while (walker)
        {
            if (!emplaceBack(walker->element))
            {
                clear();

                return *this;
            }

            walker = walker->next;
        }

        //

    	return *this;
    }

    List& operator= (List&& other)
    {
    	clear();

    	//

        front = other.front;
        back = other.back;
        listSize = other.listSize;

        other.front = nullptr;
        other.back = nullptr;
        other.listSize = 0;

        //

    	return *this;
    }

    ~List()
    {
        clear();
    }

    void clear()
    {
        auto toDelete = front;
        auto nextToDelete = nullptr;

        while (toDelete)
        {
            nextToDelete = toDelete->next;

            delete toDelete;

            listSize--;

            toDelete = nextToDelete;
        }

        front = nullptr;
        back = nullptr;
    }

    VkBool32 emplaceBack(const V& value)
    {
        auto newElement = new ListElement<V>(value);

        if (!newElement)
        {
            return VK_FALSE;
        }

        if (front == nullptr)
        {
            front = newElement;
            back = newElement;
        }
        else
        {
            back->next = newElement;
            newElement->prev = back;

            back = newElement;
        }

        listSize++;

        return VK_TRUE;
    }

    VkBool32 emplaceFront(const V& value)
    {
        auto newElement = new ListElement<V>(value);

        if (!newElement)
        {
            return VK_FALSE;
        }

        if (front == nullptr)
        {
            front = newElement;
            back = newElement;
        }
        else
        {
            front->prev = newElement;
            newElement->next = front;

            front = newElement;
        }

        listSize++;

        return VK_TRUE;
    }

    VkBool32 insert(const size_t index, const V& value)
    {
        auto walker = front;
        auto counter = 0;

        while (walker && counter + 1 < index)
        {
            walker = walker->next;

            counter++;
        }

        if (counter != index)
        {
            return VK_FALSE;
        }

        if (!walker)
        {
            return emplaceBack(value);
        }

        if (!walker->prev)
        {
            return emplaceFront(value);
        }

        auto newElement = new ListElement<V>(value);

        if (!newElement)
        {
            return VK_FALSE;
        }

        newElement->prev = walker->prev;
        walker->prev = newElement;

        newElement->prev->next = newElement;
        newElement->next = walker;

        listSize++;

        return VK_TRUE;
    }

    VkBool32 remove(const V& value)
    {
        auto walker = front;

        while (walker)
        {
            if (walker->element == value)
            {
                if (!walker->prev && !walker->next)
                {
                    front = nullptr;
                    back = nullptr;

                    delete walker;

                    listSize--;

                    return VK_TRUE;
                }
                else if (!walker->prev)
                {
                    front = walker->next;
                    front->next->prev = front;

                    delete walker;

                    listSize--;

                    return VK_TRUE;
                }
                else if (!walker->next)
                {
                    back = walker->prev;
                    back->prev->next = back;

                    delete walker;

                    listSize--;

                    return VK_TRUE;
                }
                else
                {
                    walker->prev->next = walker->next;
                    walker->next->prev = walker->prev;

                    delete walker;

                    listSize--;

                    return VK_TRUE;
                }
            }

            walker = walker->next;
        }

        return VK_FALSE;
    }

    V removeFront()
    {
    	if (!front)
    	{
    		throw std::out_of_range("No such element");
    	}

		auto toDelete = front;

		front = front->next;

		if (front)
		{
			front->prev = nullptr;
		}

		auto result = toDelete->element;

		delete toDelete;

		listSize--;

		return result;
    }

    V removeBack()
    {
    	if (!back)
    	{
    		throw std::out_of_range("No such element");
    	}

		auto toDelete = back;

		back = back->prev;

		if (back)
		{
			back->next = nullptr;
		}

		auto result = toDelete->element;

		delete toDelete;

		listSize--;

		return result;
    }

    V getFront()
    {
    	if (!front)
    	{
    		throw std::out_of_range("No such element");
    	}

        return front->value;
    }

    V getBack()
    {
    	if (!back)
    	{
    		throw std::out_of_range("No such element");
    	}

        return back->value;
    }

    VkBool32 contains(const V& value) const
    {
        auto walker = front;

        while (walker)
        {
            if (walker->element == value)
            {
                return VK_TRUE;
            }

            walker = walker->next;
        }

        return VK_FALSE;
    }

    V& operator[](const size_t index)
    {
    	if (index > listSize)
    	{
    		throw std::out_of_range(std::to_string(index) + " > " + std::to_string(listSize));
    	}
    	else if (index == listSize)
    	{
    		// Allow to append at the end.
    		V v;

    		emplaceBack(v);
    	}

        auto walker = front;
        auto counter = 0;

        while (walker && counter + 1 < index)
        {
            walker = walker->next;

            counter++;
        }

        return walker->element;
    }

    const V& operator[](const size_t index) const
    {
    	if (index >= listSize)
    	{
    		throw std::out_of_range(std::to_string(index) + " >= " + std::to_string(listSize));
    	}

        auto walker = front;
        auto counter = 0;

        while (walker && counter + 1 < index)
        {
            walker = walker->next;

            counter++;
        }

        return walker->element;
    }

    size_t size() const
    {
        return listSize;
    }
};

}

#endif /* VKTS_LIST_HPP_ */
