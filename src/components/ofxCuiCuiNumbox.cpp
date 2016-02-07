/*
 * ofxCuiCuiNumbox.cpp
 *
 * Number box (with mouse + keyboard interaction, C74 max style)
 *
 * This code has been authored by Jules Françoise <jfrancoi@sfu.ca>
 * during his postdoc contract at Simon Fraser University (MovingStories
 * project).
 * See:
 *    http://julesfrancoise.com/
 *    http://movingstories.ca/
 *
 * Copyright (C) 2016 Jules Francoise.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "ofxCuiCuiNumbox.hpp"

ofxCuiCui::Numbox::Numbox(int x_, int y_, int width_, int height_)
    : ofxCuiCui::Component(x_, y_, width_, height_),
      value(0.),
      min(std::numeric_limits<float>::lowest()),
      max(std::numeric_limits<float>::max()),
      precision(2),
      drag_step_(pow(10, -precision)),
      dragging_(false) {
    type_ = ofxCuiCui::Type::Numbox;
    shape = ofxCuiCui::Shape::Rectangle;
    font.alignment = ofxCuiCui::Anchor::TopCenter;
    value_text_.alignment = ofxCuiCui::Anchor::MiddleCenter;
    ofAddListener(ofEvents().mousePressed, this,
                  &ofxCuiCui::Numbox::mousePressed);
    ofAddListener(ofEvents().mouseReleased, this,
                  &ofxCuiCui::Numbox::mouseReleased);
    ofAddListener(ofEvents().mouseDragged, this,
                  &ofxCuiCui::Numbox::mouseDragged);
    ofAddListener(ofEvents().keyPressed, this, &ofxCuiCui::Numbox::keyPressed);
}

ofxCuiCui::Numbox::~Numbox() {
    ofRemoveListener(ofEvents().mousePressed, this,
                     &ofxCuiCui::Numbox::mousePressed);
    ofRemoveListener(ofEvents().mouseReleased, this,
                     &ofxCuiCui::Numbox::mouseReleased);
    ofRemoveListener(ofEvents().mouseDragged, this,
                     &ofxCuiCui::Numbox::mouseDragged);
    ofRemoveListener(ofEvents().keyPressed, this,
                     &ofxCuiCui::Numbox::keyPressed);
}

void ofxCuiCui::Numbox::update() {
    ofxCuiCui::Component::update();
    value_text_.color = font.color;
    value_text_.text = user_input_;
    if (user_input_.size() == 0) {
        std::stringstream str;
        str << fixed << setprecision(precision) << value;
        value_text_.text = str.str();
    }
    value_text_.x = x;
    value_text_.y = font.rect.y + font.rect.height;
    value_text_.width = width;
    value_text_.height = height - font.rect.height;
    value_text_.update();
}

void ofxCuiCui::Numbox::clipValue() {
    value = drag_step_ * round(value / drag_step_);
    value = (value > max) ? max : (value < min) ? min : value;
}

void ofxCuiCui::Numbox::drawLabel() {
    ofxCuiCui::Component::drawLabel();
    if (active) {
        ofFill();
        ofSetColor(current_frame_color_, 50);
        ofDrawRectangle(value_text_.rect.x - 10, value_text_.rect.y - 10,
                        value_text_.rect.width + 20,
                        value_text_.rect.height + 20);
    }
    value_text_.draw();
}

void ofxCuiCui::Numbox::mouseDragged(ofMouseEventArgs &e) {
    if (disabled) return;
    dragging_ = true;
    hover_ = inside(e.x, e.y);
    if (!active) return;
    float distance = e.y - y_pressed_;
    value = value_pressed_ - round(distance / 5.) * drag_step_;
    clipValue();
    ofxCuiCui::Numbox::ValueEvent event =
        ofxCuiCui::Numbox::ValueEvent(this, value);
    if (numbox_event_callback_ != nullptr) {
        numbox_event_callback_(event);
    }
}

void ofxCuiCui::Numbox::mousePressed(ofMouseEventArgs &e) {
    if (disabled) return;
    if (inside(e.x, e.y) && e.button == 0) {
        active = true;
        y_pressed_ = e.y;
        value_pressed_ = value;
        user_input_ = "";
        int characterPressedIndex = value_text_.getCharacterIndex(e.x, e.y);
        if (characterPressedIndex >= 0) {
            int dotIndex = std::max(1, int(log10(value)) + 1);
            if (characterPressedIndex < dotIndex) {
                drag_step_ = pow(10, dotIndex - characterPressedIndex - 1);
            } else {
                drag_step_ = pow(10, dotIndex - characterPressedIndex);
            }
        } else {
            drag_step_ = pow(10, -precision);
        }
    } else {
        active = false;
    }
}

void ofxCuiCui::Numbox::mouseReleased(ofMouseEventArgs &e) {
    if (disabled) return;
    if (dragging_) {
        active = false;
    }
    dragging_ = false;
}

void ofxCuiCui::Numbox::keyPressed(ofKeyEventArgs &e) {
    if (disabled) return;
    if (!active) return;
    if (e.key == OF_KEY_BACKSPACE || e.key == OF_KEY_DEL) {
        user_input_.pop_back();
    } else if (e.key == OF_KEY_RETURN || e.key == OF_KEY_TAB) {
        value = std::stof(user_input_);
        clipValue();
        ofxCuiCui::Numbox::ValueEvent event =
            ofxCuiCui::Numbox::ValueEvent(this, value);
        if (numbox_event_callback_ != nullptr) {
            numbox_event_callback_(event);
        }
        active = false;
        user_input_ = "";
    } else if (e.key == 46) {
        user_input_ += ".";
    } else if (e.key == 45) {
        user_input_ += "-";
    } else if (e.key >= 48 && e.key <= 57) {
        user_input_ += static_cast<char>(e.key);
    } else if (e.key == OF_KEY_DOWN || e.key == OF_KEY_LEFT) {
        value -= drag_step_;
        clipValue();
        ofxCuiCui::Numbox::ValueEvent event =
            ofxCuiCui::Numbox::ValueEvent(this, value);
        if (numbox_event_callback_ != nullptr) {
            numbox_event_callback_(event);
        }
    } else if (e.key == OF_KEY_UP || e.key == OF_KEY_RIGHT) {
        value += drag_step_;
        clipValue();
        ofxCuiCui::Numbox::ValueEvent event =
            ofxCuiCui::Numbox::ValueEvent(this, value);
        if (numbox_event_callback_ != nullptr) {
            numbox_event_callback_(event);
        }
    }
}

ofxCuiCui::Numbox::ValueEvent::ValueEvent(Numbox *sender_, float value_)
    : sender(sender_), value(value_) {}
