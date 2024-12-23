import time
import torch
from transformers import AutoTokenizer, AutoModelForCausalLM
import sys

def load_model_and_tokenizer(model_name):
    tokenizer = AutoTokenizer.from_pretrained(model_name)
    model = AutoModelForCausalLM.from_pretrained(model_name)
    device = "cuda" if torch.cuda.is_available() else "cpu"
    model.to(device)
    return tokenizer, model, device

def read_prompt_from_file(filename):
    with open(filename, 'r') as file:
        return file.read()

def generate_text(tokenizer, model, device, prompt, max_length=200, temperature=0.7, top_p=0.9):
    input_ids = tokenizer.encode(prompt, return_tensors="pt").to(device)
    output = model.generate(input_ids, max_length=max_length, pad_token_id=tokenizer.eos_token_id, no_repeat_ngram_size=3, num_return_sequences=1, top_p=top_p, temperature=temperature, do_sample=True)
    generated_text = tokenizer.decode(output[0], skip_special_tokens=True)
    return generated_text

def write_generated_text_to_file(filename, generated_text, prompt):
    generated_text = generated_text.replace(prompt, "")
    with open(filename, 'w') as file:
        generated_text = "gpt2bot> " + generated_text
        file.write(generated_text)

def main():
    start = time.time()
    model_name = "openai-community/gpt2-medium"
    tokenizer, model, device = load_model_and_tokenizer(model_name)
    print(f'Device: {device}')

    uuid = sys.argv[1]
    filename ='chatbot_details_'+uuid+".txt"
    prompt = read_prompt_from_file(filename)

    context_prompt = ("You are a Chatbot capable of answering questions for airline services. "
                      "Please respond to the following user question to the best of your knowledge "
                      "without generating any follow-up questions and dont just try to complete my sentence. : ")
    prompt = context_prompt + prompt

    generated_text = generate_text(tokenizer, model, device, prompt)

    end = time.time()
    write_generated_text_to_file(filename, generated_text, prompt)
    print(f'Time taken: {end - start} seconds')

if __name__ == "__main__":
    main()
