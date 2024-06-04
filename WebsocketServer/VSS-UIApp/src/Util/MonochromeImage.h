#pragma once

#include <vector>

// store 1 pixel on 1 bit
// this is used to know if a button should trigger "OnClick" even when a button does not have a square format

class MonochromeImage 
{
private:
    int m_dWidth;
    int m_dHeight;
    std::vector<unsigned char> m_data;

    // Function to get the index of the bit representing the pixel at (x, y)
    inline int getIndex(int x, int y) const 
	{
        return y * ((m_dWidth + 7) / 8) + x / 8;
    }

    // Function to get the bit position within a byte for a pixel at (x, y)
    inline int getBitPosition(int x) const 
	{
        return 7 - (x % 8);
    }

public:
    MonochromeImage()
    {
        m_dWidth = 0;
        m_dHeight = 0;
    }
    void Init(int w, int h)
	{
        m_dWidth = w;
        m_dHeight = h;
        // Calculate the number of bytes needed for the image data
        int dataSize = ((w + 7) / 8) * h;
        // Resize the data vector and initialize with all bits set to 0
        m_data.resize(dataSize, 0);
    }

    // Function to set the value of a pixel at (x, y)
    void setPixel(int x, int y, bool value) 
	{
        if (x < 0 || x >= m_dWidth || y < 0 || y >= m_dHeight)
		{
			return;
//            throw std::out_of_range("Pixel coordinates out of range");
        }
        int index = getIndex(x, y);
        int bitPos = getBitPosition(x);
        if (value) 
		{
            m_data[index] |= (1 << bitPos); // Set the bit
        } 
		else 
		{
            m_data[index] &= ~(1 << bitPos); // Clear the bit
        }
    }

    // Function to get the value of a pixel at (x, y)
    inline bool getPixel(int x, int y) const 
	{
        // non loaded state
        if (m_dWidth == 0)
        {
            return true;
        }
        // out of bounds check ?
        if (x < 0 || x >= m_dWidth || y < 0 || y >= m_dHeight)
		{
			return false;
//            throw std::out_of_range("Pixel coordinates out of range");
        }
        int index = getIndex(x, y);
        int bitPos = getBitPosition(x);
        return (m_data[index] >> bitPos) & 1; // Get the bit value
    }

    void ConstructFromSTBImage(const unsigned char* imageData, int imgWidth, int imgHeight) 
    {
        if (imgWidth != m_dWidth || imgHeight != m_dHeight)
        {
            return;
//            throw std::invalid_argument("Image dimensions do not match");
        }
        // Process alpha channel
        for (int y = 0; y < m_dHeight; ++y)
        {
            int rowIndexBaseY = y * ((m_dWidth + 7) / 8);
            for (int x = 0; x < m_dWidth; ++x)
            {
                int alpha = imageData[(y * m_dWidth + x) * 4 + 3];
                if (alpha != 0) 
                {
                    int index = rowIndexBaseY + x / 8;
                    int bitPos = 7 - (x % 8);
                    m_data[index] |= (1 << bitPos); // Set the bit
                }
            }
        }
    }
};