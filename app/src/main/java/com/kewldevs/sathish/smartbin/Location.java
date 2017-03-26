
package com.kewldevs.sathish.smartbin;



public class Location {

    private String mLat;
    private String mLng;

    Location()
    {

    }

    public String getLat() {
        return mLat;
    }

    public void setLat(String lat) {
        mLat = lat;
    }

    public String getLng() {
        return mLng;
    }

    public void setLng(String lng) {
        mLng = lng;
    }

    @Override
    public String toString() {
        return "Location{" +
                "mLat='" + mLat + '\'' +
                ", mLng='" + mLng + '\'' +
                '}';
    }
}
