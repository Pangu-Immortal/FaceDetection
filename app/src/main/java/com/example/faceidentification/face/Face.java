package com.example.faceidentification.face;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

public class Face {
    private int Id;
    private String Name;
    private String FileName;

    private JSONObject jsonObject;

    private FaceShapes[] faceFeatures;

    private  double[] faceDescriptors;

    public Face(FaceShapes[] faceFeatures) throws JSONException {
        Name = "";
        FileName = "";
        this.faceFeatures = faceFeatures;
        jsonObject = new JSONObject();
        jsonObject.put("name",null);
        jsonObject.put("file",null);
        JSONArray features = new JSONArray();
        for(int i = 0; i < faceFeatures.length; ++i) {
            features.put(faceFeatures[i].getJsonObject());
        }
        jsonObject.put("features",features);
    }

    public Face(double[] faceDescriptors) throws JSONException {
        this.faceDescriptors = faceDescriptors;
        jsonObject = new JSONObject();
        jsonObject.put("name","");
        jsonObject.put("file","");
        JSONArray faceDescriptor = new JSONArray();
        for(int i = 0; i < faceDescriptors.length; ++i) {
            faceDescriptor.put(faceDescriptors[i]);
        }
        jsonObject.put("descriptors",faceDescriptor);
    }

    public Face(String name, String fileName) throws JSONException {
        Id = 0;
        Name = name;
        FileName = fileName;
        jsonObject = new JSONObject();
        //jsonObject.put("id",id);
        jsonObject.put("name",name);
        jsonObject.put("file",fileName);
    }



    public Face() {
        Id = 0;
        Name = "";
        FileName = "";
        jsonObject = new JSONObject();
    }

    public int getId() {
        return Id;
    }

    public void setId(int id) throws JSONException {
        //jsonObject.put("id",id);
        Id = id;
    }

    public String getName() {
        return Name;
    }

    public void setName(String name) throws JSONException {
        jsonObject.put("name",name);
        Name = name;
    }

    public String getFileName() {
        return FileName;
    }

    public void setFileName(String fileName) throws JSONException {
        jsonObject.put("file",fileName);
        FileName = fileName;
    }

    public String getJsonStr() {
        return jsonObject.toString();
    }

    public void setJsonObject(JSONObject jsonObj) throws JSONException {
        jsonObject = jsonObj;
        this.Id = jsonObj.getInt("Id");
        this.Name = jsonObj.getString("name");
        this.FileName = jsonObj.getString("file");

    }

    public FaceShapes[] getFaceFeature() {
        return this.faceFeatures;
    }
}
