package com.example.faceidentification.face;

import org.json.JSONException;
import org.json.JSONObject;

public class FaceShapes {
    public int x = 0;
    public int y = 0;

    public FaceShapes(int x, int y) {
        this.x = x;
        this.y = y;
    }

    JSONObject getJsonObject() throws JSONException {
        JSONObject jsonObject = new JSONObject();
        jsonObject.put("x",x);
        jsonObject.put("y",y);
        return jsonObject;
    }

    public FaceShapes() {

    }

}
